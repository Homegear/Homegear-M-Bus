/* Copyright 2013-2019 Homegear GmbH */

#include "../Gd.h"
#include "Hgdc.h"

#define CMD_DATA_IND 0x03
#define CMD_RESET_REQ 0x05
#define CMD_SET_REQ 0x09
#define CMD_GET_REQ 0x0A

namespace Mbus {

Hgdc::Hgdc(std::shared_ptr<BaseLib::Systems::PhysicalInterfaceSettings> settings) : IMbusInterface(settings) {
  _settings = settings;
  _out.init(Gd::bl);
  _out.setPrefix(Gd::out.getPrefix() + "HGDC \"" + settings->id + "\": ");

  signal(SIGPIPE, SIG_IGN);

  _stopped = true;

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

Hgdc::~Hgdc() {
  stopListening();
  _bl->threadManager.join(_initThread);
}

void Hgdc::startListening() {
  try {
    Gd::bl->hgdc->unregisterPacketReceivedEventHandler(_packetReceivedEventHandlerId);

    std::string settingName = "mode";
    auto modeSetting = Gd::family->getFamilySetting(settingName);
    if (modeSetting) _settings->mode = BaseLib::HelperFunctions::toLower(modeSetting->stringValue);
    if (_settings->mode.empty() || (_settings->mode != "t" && _settings->mode != "s" && _settings->mode != "c")) {
      _out.printError("Warning: \"Mode\" is not set or invalid in \"mbus.conf\". Setting it to \"T\".");
      _settings->mode = "t";
    }

    _packetReceivedEventHandlerId = Gd::bl->hgdc->registerPacketReceivedEventHandler(MY_FAMILY_ID,
                                                                                     std::function<void(int64_t, const std::string &, const std::vector<uint8_t> &)>(std::bind(&Hgdc::processPacket,
                                                                                                                                                                               this,
                                                                                                                                                                               std::placeholders::_1,
                                                                                                                                                                               std::placeholders::_2,
                                                                                                                                                                               std::placeholders::_3)));
    IPhysicalInterface::startListening();

    _stopped = false;
    init();
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Hgdc::stopListening() {
  try {
    _stopped = true;
    IPhysicalInterface::stopListening();
    Gd::bl->hgdc->unregisterPacketReceivedEventHandler(_packetReceivedEventHandlerId);
    _packetReceivedEventHandlerId = -1;
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Hgdc::init() {
  try {
    _initComplete = false;

    if (_settings->type == "hgdc1700") {
      std::vector<uint8_t> parameters;
      std::vector<uint8_t> response;
      //{{{ Query all parameters
      for (int32_t i = 0; i < 5; i++) {
        std::vector<uint8_t> data{0xFF, CMD_GET_REQ, 0x02, 0x00, 80, 0x00};
        addAmberCrc8(data);
        GetSerialResponse(data, response);
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

      if (parameters.at(10) != 128) {
        settingsChanged = true;
        _out.printInfo("Info: Setting APP_MAXPacketLength from 0x" + BaseLib::HelperFunctions::getHexString((int32_t)parameters.at(10), 2) + " to 128");
        setParameter(10, 128);
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
        _out.printInfo("Info: Setting RF_AutoSleep from 0x" + BaseLib::HelperFunctions::getHexString((int32_t)parameters.at(63), 2) + " to 0");
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
          addAmberCrc8(data);
          GetSerialResponse(data, response);
          if (response.size() != 5 || response[3] != 0) {
            if (i < 4) continue;
            _out.printError("Error executing CMD_RESET_REQ on device. Response was: " + BaseLib::HelperFunctions::getHexString(response));
            _stopped = true;
            return;
          }
          break;
        }
      }
    }

    _out.printInfo("Init complete (device type: " + _settings->type + ").");

    _initComplete = true;
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

bool Hgdc::setParameter(uint8_t address, uint8_t value) {
  try {
    std::vector<uint8_t> response;
    for (int32_t i = 0; i < 5; i++) {
      std::vector<uint8_t> data{0xFF, CMD_SET_REQ, 0x03, address, 0x01, value, 0x00};
      addAmberCrc8(data);
      GetSerialResponse(data, response);
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

void Hgdc::RawSend(std::vector<uint8_t> &packet) {
  try {
    if (!Gd::bl->hgdc->sendPacket(_settings->serialNumber, packet)) {
      _out.printError("Error sending packet " + BaseLib::HelperFunctions::getHexString(packet) + ".");
    }
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Hgdc::processPacket(int64_t familyId, const std::string &serialNumber, const std::vector<uint8_t> &data) {
  try {
    if (serialNumber != _settings->serialNumber) return;

    if (data.size() < 5) {
      _out.printInfo("Info: Too small packet received: " + BaseLib::HelperFunctions::getHexString(data));
      return;
    }

    uint8_t crc8 = 0;
    for (uint32_t i = 0; i < data.size() - 1; i++) {
      crc8 = crc8 ^ (uint8_t)data[i];
    }
    if (crc8 != data.back()) {
      _out.printInfo("Info: CRC failed for packet: " + BaseLib::HelperFunctions::getHexString(data));
      return;
    }

    _lastPacketReceived = BaseLib::HelperFunctions::getTime();

    uint8_t packetType = data[1];

    std::unique_lock<std::mutex> requestsGuard(requests_mutex_);
    auto requestIterator = requests_.find(packetType);
    if (requestIterator != requests_.end()) {
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

}