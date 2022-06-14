/* Copyright 2013-2019 Homegear GmbH */

#include "../Gd.h"
#include "Amber.h"

#define CMD_DATA_IND 0x03
#define CMD_RESET_REQ 0x05
#define CMD_SET_REQ 0x09
#define CMD_GET_REQ 0x0A

namespace Mbus {

Amber::Amber(std::shared_ptr<BaseLib::Systems::PhysicalInterfaceSettings> settings) : IMbusInterface(settings) {
  _initComplete = false;

  _settings = settings;
  _out.init(Gd::bl);
  _out.setPrefix(Gd::out.getPrefix() + "Amber \"" + settings->id + "\": ");

  signal(SIGPIPE, SIG_IGN);

  if (_settings->baudrate == -1) _settings->baudrate = 9600;

  std::string settingName = "securitymodewhitelist";
  auto setting = Gd::family->getFamilySetting(settingName);
  if (setting) {
    auto elements = BaseLib::HelperFunctions::splitAll(setting->stringValue, ',');
    for (auto &element: elements) {
      BaseLib::HelperFunctions::trim(element);
      int32_t mode = BaseLib::Math::getNumber(element);
      _securityModeWhitelist.emplace(mode);
      Gd::out.printInfo("Info: Adding mode " + std::to_string(mode) + " to security mode whitelist");
    }
  }

  //0: No encryption
  //1: Reserved
  //2: DES encryption with CBC; IV is zero (deprecated)
  //3: DES encryption with CBC; IV is not zero (deprecated)
  //4: AES-CBC-128; IV = 0
  //5: AES-CBC-128; IV != 0
  //6: Reserved
  //7: AES-CBC-128; IV=0; KDF
  //8: AES-CTR-128; CMAC
  //9: AES-GCM-128
  //10: AES-CCM-128
  //11: Reserved
  //12: Reserved
  //13: Specific usage
  //14: Reserved
  //15: Specific usage
  //16 - 31: Reserved
  if (_securityModeWhitelist.find(0) != _securityModeWhitelist.end() ||
      _securityModeWhitelist.find(2) != _securityModeWhitelist.end() ||
      _securityModeWhitelist.find(3) != _securityModeWhitelist.end() ||
      _securityModeWhitelist.find(4) != _securityModeWhitelist.end() ||
      _securityModeWhitelist.find(5) != _securityModeWhitelist.end()) {
    Gd::out.printWarning("Warning: Your security mode whitelist contains insecure security modes. This is a potential risk.");
  }
}

Amber::~Amber() {
  stopListening();
  Gd::bl->threadManager.join(_initThread);
}

void Amber::setup(int32_t userID, int32_t groupID, bool setPermissions) {
  try {
    if (setPermissions) setDevicePermission(userID, groupID);
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Amber::startListening() {
  try {
    stopListening();

    if (_settings->device.empty()) {
      _out.printError("Error: No device defined. Please specify it in \"mbus.conf\".");
      return;
    }

    BaseLib::HelperFunctions::toLower(_settings->mode);
    if (_settings->mode.empty() || (_settings->mode != "t" && _settings->mode != "s" && _settings->mode != "c")) {
      _out.printError("Warning: \"Mode\" is not set or invalid in \"mbus.conf\". Setting it to \"T\".");
      _settings->mode = "t";
    }

    _out.printInfo("Info: Opening device " + _settings->device + ". Baudrate set to " + std::to_string(_settings->baudrate) + ".");

    _serial.reset(new BaseLib::SerialReaderWriter(_bl, _settings->device, _settings->baudrate, 0, true, -1));
    _serial->openDevice(false, false, false);
    if (!_serial->isOpen()) {
      _out.printError("Error: Could not open device.");
      return;
    }

    _stopCallbackThread = false;
    _stopped = false;
    int32_t result = 0;
    char byte = 0;
    while (result == 0) {
      //Clear buffer, otherwise the first response cannot be sent by the module if the buffer is full.
      result = _serial->readChar(byte, 100000);
    }
    if (_settings->listenThreadPriority > -1) _bl->threadManager.start(_listenThread, true, _settings->listenThreadPriority, _settings->listenThreadPolicy, &Amber::listen, this);
    else _bl->threadManager.start(_listenThread, true, &Amber::listen, this);
    IPhysicalInterface::startListening();

    init();
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Amber::stopListening() {
  try {
    _stopCallbackThread = true;
    _bl->threadManager.join(_listenThread);
    _stopped = true;
    _initComplete = false;
    if (_serial) _serial->closeDevice();
    IPhysicalInterface::stopListening();
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

bool Amber::setParameter(uint8_t address, uint8_t value) {
  try {
    std::vector<uint8_t> response;
    for (int32_t i = 0; i < 5; i++) {
      std::vector<uint8_t> data{0xFF, CMD_SET_REQ, 0x03, address, 0x01, value, 0x00};
      addCrc8(data);
      getResponse(data, response);
      if (response.size() != 5 || response[3] != 0) {
        if (i < 4) continue;
        _out.printError("Error executing CMD_SET_REQ on device. Response was: " + BaseLib::HelperFunctions::getHexString(response));
        _stopped = true;
        return false;
      }
      break;
    }

    return true;
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

void Amber::init() {
  try {
    std::vector<uint8_t> parameters;
    std::vector<uint8_t> response;
    //{{{ Query all parameters
    for (int32_t i = 0; i < 5; i++) {
      std::vector<uint8_t> data{0xFF, CMD_GET_REQ, 0x02, 0x00, 80, 0x00};
      addCrc8(data);
      getResponse(data, response);
      if (response.size() != 86 || response[3] != 0 || response[4] != 80) {
        if (i < 4) continue;
        _out.printError("Error executing CMD_GET_REQ on device. Response was: " + BaseLib::HelperFunctions::getHexString(response));
        _stopped = true;
        return;
      }
      parameters.insert(parameters.end(), response.begin() + 5, response.end() - 1);
      break;
    }
    //}}}

    if (parameters.size() != 80) return;

    bool settingsChanged = false;
    if (parameters.at(5) != 1) {
      settingsChanged = true;
      _out.printInfo("Info: Setting UART_CMD_Out_Enable from 0x" + BaseLib::HelperFunctions::getHexString((int32_t)parameters.at(5), 2) + " to 1");
      setParameter(5, 1);
    }

    if (parameters.at(10) != 250) {
      settingsChanged = true;
      _out.printInfo("Info: Setting APP_MAXPacketLength from 0x" + BaseLib::HelperFunctions::getHexString((int32_t)parameters.at(10), 2) + " to 250");
      setParameter(10, 250);
    }

    if (parameters.at(11) != 0) {
      settingsChanged = true;
      _out.printInfo("Info: Setting APP_AES_Enable from 0x" + BaseLib::HelperFunctions::getHexString((int32_t)parameters.at(11), 2) + " to 0");
      setParameter(11, 0);
    }

    if (parameters.at(43) != 0) {
      settingsChanged = true;
      _out.printInfo("Info: Setting MBUS_RXTimeout from 0x" + BaseLib::HelperFunctions::getHexString((int32_t)parameters.at(43), 2) + " to 0");
      setParameter(43, 0);
    }

    if (parameters.at(44) != 0) //Not needed for all wM-Bus modes. S and T always use format A.
    {
      settingsChanged = true;
      _out.printInfo("Info: Setting MBUS_FrameFormat from 0x" + BaseLib::HelperFunctions::getHexString((int32_t)parameters.at(44), 2) + " to 0");
      setParameter(44, 0);
    }

    if (parameters.at(61) != 6) {
      settingsChanged = true;
      _out.printInfo("Info: Setting RF_Power from 0x" + BaseLib::HelperFunctions::getHexString((int32_t)parameters.at(61), 2) + " to 6");
      setParameter(61, 6);
    }

    if (parameters.at(63) != 0) {
      settingsChanged = true;
      _out.printInfo("Info: Setting RF_AutoSleep from 0x" + BaseLib::HelperFunctions::getHexString((int32_t)parameters.at(63), 2) + " to 6");
      setParameter(63, 0);
    }

    if (parameters.at(69) != 1) {
      settingsChanged = true;
      _out.printInfo("Info: Setting RSSI_Enable from 0x" + BaseLib::HelperFunctions::getHexString((int32_t)parameters.at(69), 2) + " to 1");
      setParameter(69, 1);
    }

    if (_settings->mode == "c") {
      if (parameters.at(70) != 0x0E) {
        settingsChanged = true;
        _out.printInfo("Info: Setting Mode_Preselect from 0x" + BaseLib::HelperFunctions::getHexString((int32_t)parameters.at(70), 2) + " to 0x0E (C2 other)");
        setParameter(70, 0x0E);
      }
    } else if (_settings->mode == "t") {
      if (parameters.at(70) != 0x08) {
        settingsChanged = true;
        _out.printInfo("Info: Setting Mode_Preselect from 0x" + BaseLib::HelperFunctions::getHexString((int32_t)parameters.at(70), 2) + " to 0x0E (T2 other)");
        setParameter(70, 0x08);
      }
    } else if (_settings->mode == "s") {
      if (parameters.at(70) != 0x03) {
        settingsChanged = true;
        _out.printInfo("Info: Setting Mode_Preselect from 0x" + BaseLib::HelperFunctions::getHexString((int32_t)parameters.at(70), 2) + " to 0x0E (S2)");
        setParameter(70, 0x03);
      }
    }

    if (settingsChanged) {
      //Reset
      for (int32_t i = 0; i < 5; i++) {
        std::vector<uint8_t> data{0xFF, CMD_RESET_REQ, 0x00, 0x00};
        addCrc8(data);
        getResponse(data, response);
        if (response.size() != 5 || response[3] != 0) {
          if (i < 4) continue;
          _out.printError("Error executing CMD_RESET_REQ on device. Response was: " + BaseLib::HelperFunctions::getHexString(response));
          _stopped = true;
          return;
        }
        break;
      }
    }

    _out.printInfo("Init complete.");

    _initComplete = true;
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Amber::reconnect() {
  try {
    _serial->closeDevice();
    _initComplete = false;
    _serial->openDevice(false, false, false);
    if (!_serial->isOpen()) {
      _out.printError("Error: Could not open device.");
      return;
    }
    _stopped = false;

    Gd::bl->threadManager.join(_initThread);
    _bl->threadManager.start(_initThread, true, &Amber::init, this);
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Amber::listen() {
  try {
    std::vector<uint8_t> data;
    data.reserve(255);
    char byte = 0;
    int32_t result = 0;
    uint32_t size = 0;
    uint8_t crc8 = 0;

    while (!_stopCallbackThread) {
      try {
        if (_stopped || !_serial || !_serial->isOpen()) {
          if (_stopCallbackThread) return;
          if (_stopped) _out.printWarning("Warning: Connection to device closed. Trying to reconnect...");
          _serial->closeDevice();
          std::this_thread::sleep_for(std::chrono::milliseconds(10000));
          reconnect();
          continue;
        }

        result = _serial->readChar(byte, 100000);
        if (result == -1) {
          _out.printError("Error reading from serial device.");
          _stopped = true;
          size = 0;
          data.clear();
          continue;
        } else if (result == 1) {
          if (!data.empty()) _out.printWarning("Warning: Incomplete packet received: " + BaseLib::HelperFunctions::getHexString(data));
          size = 0;
          data.clear();
          continue;
        }

        if (data.empty() && (uint8_t)byte != 0xFF) continue;
        data.push_back((uint8_t)byte);

        if (size == 0 && data.size() == 3) {
          size = data.at(2) + 4;
        }
        if (size > 0 && data.size() == size) {
          crc8 = 0;
          for (uint32_t i = 0; i < data.size() - 1; i++) {
            crc8 = crc8 ^ (uint8_t)data[i];
          }
          if (crc8 != data.back()) {
            _out.printInfo("Info: CRC failed for packet: " + BaseLib::HelperFunctions::getHexString(data));
            size = 0;
            data.clear();
            continue;
          }

          processPacket(data);

          _lastPacketReceived = BaseLib::HelperFunctions::getTime();
          size = 0;
          data.clear();
        }
      }
      catch (const std::exception &ex) {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
      }
    }
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Amber::processPacket(std::vector<uint8_t> &data) {
  try {
    if (data.size() < 5) {
      _out.printInfo("Info: Too small packet received: " + BaseLib::HelperFunctions::getHexString(data));
      return;
    }

    uint8_t packetType = data[1];

    std::unique_lock<std::mutex> requestsGuard(_requestsMutex);
    std::map<uint8_t, std::shared_ptr<Request>>::iterator requestIterator = _requests.find(packetType);
    if (requestIterator != _requests.end()) {
      std::shared_ptr<Request> request = requestIterator->second;
      requestsGuard.unlock();
      request->response = data;
      {
        std::lock_guard<std::mutex> lock(request->mutex);
        request->mutexReady = true;
      }
      request->conditionVariable.notify_one();
      return;
    } else requestsGuard.unlock();

    if (data.at(1) == CMD_DATA_IND) {
      PMbusPacket packet = std::make_shared<MbusPacket>(data);
      if (packet->headerValid()) {
        if (_securityModeWhitelist.find(packet->getEncryptionMode()) != _securityModeWhitelist.end()) {
          raisePacketReceived(packet);
        } else _out.printWarning("Warning: Dropping packet, because security mode " + std::to_string(packet->getEncryptionMode()) + " is not in whitelist: " + BaseLib::HelperFunctions::getHexString(data));
      } else _out.printWarning("Warning: Could not parse packet: " + BaseLib::HelperFunctions::getHexString(data));
    } else _out.printWarning("Warning: Not processing packet: " + BaseLib::HelperFunctions::getHexString(data));
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Amber::RawSend(std::vector<uint8_t> &packet) {
  try {
    if (!_serial || !_serial->isOpen()) return;
    _serial->writeData(packet);
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

}
