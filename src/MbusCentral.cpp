/* Copyright 2013-2019 Homegear GmbH */

#include "MbusCentral.h"
#include "GD.h"

#include <iomanip>

namespace Mbus {

MbusCentral::MbusCentral(ICentralEventSink *eventHandler) : BaseLib::Systems::ICentral(MY_FAMILY_ID, GD::bl, eventHandler) {
  init();
}

MbusCentral::MbusCentral(uint32_t deviceID, std::string serialNumber, ICentralEventSink *eventHandler) : BaseLib::Systems::ICentral(MY_FAMILY_ID, GD::bl, deviceID, serialNumber, -1, eventHandler) {
  init();
}

MbusCentral::~MbusCentral() {
  dispose();
}

void MbusCentral::dispose(bool wait) {
  try {
    if (_disposing) return;
    _disposing = true;
    {
      std::lock_guard<std::mutex> pairingModeGuard(_pairingModeThreadMutex);
      _stopPairingModeThread = true;
      _bl->threadManager.join(_pairingModeThread);
    }

    _stopWorkerThread = true;
    GD::out.printDebug("Debug: Waiting for worker thread of device " + std::to_string(_deviceId) + "...");
    GD::bl->threadManager.join(_workerThread);

    GD::out.printDebug("Removing device " + std::to_string(_deviceId) + " from physical device's event queue...");
    GD::interfaces->removeEventHandlers();
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void MbusCentral::init() {
  try {
    if (_initialized) return; //Prevent running init two times
    _initialized = true;
    _pairing = false;
    _stopPairingModeThread = false;
    _stopWorkerThread = false;
    _timeLeftInPairingMode = 0;

    _localRpcMethods.insert(std::pair<std::string, std::function<BaseLib::PVariable(const BaseLib::PRpcClientInfo &clientInfo, const BaseLib::PArray &parameters)>>("processPacket",
                                                                                                                                                                    std::bind(&MbusCentral::processPacket,
                                                                                                                                                                              this,
                                                                                                                                                                              std::placeholders::_1,
                                                                                                                                                                              std::placeholders::_2)));

    GD::interfaces->addEventHandlers((BaseLib::Systems::IPhysicalInterface::IPhysicalInterfaceEventSink *)this);

    GD::bl->threadManager.start(_workerThread, true, _bl->settings.workerThreadPriority(), _bl->settings.workerThreadPolicy(), &MbusCentral::worker, this);
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void MbusCentral::worker() {
  try {
    std::chrono::milliseconds sleepingTime(1000);
    uint64_t lastPeer = 0;

    while (!_stopWorkerThread && !GD::bl->shuttingDown) {
      try {
        std::this_thread::sleep_for(sleepingTime);
        if (_stopWorkerThread || GD::bl->shuttingDown) return;

        std::shared_ptr<MbusPeer> peer;

        {
          std::lock_guard<std::mutex> peersGuard(_peersMutex);
          if (!_peersById.empty()) {
            if (!_peersById.empty()) {
              std::map<uint64_t, std::shared_ptr<BaseLib::Systems::Peer>>::iterator nextPeer = _peersById.find(lastPeer);
              if (nextPeer != _peersById.end()) {
                nextPeer++;
                if (nextPeer == _peersById.end()) nextPeer = _peersById.begin();
              } else nextPeer = _peersById.begin();
              lastPeer = nextPeer->first;
              peer = std::dynamic_pointer_cast<MbusPeer>(nextPeer->second);
            }
          }
        }

        if (peer && !peer->deleting) peer->worker();
        GD::interfaces->worker();
      }
      catch (const std::exception &ex) {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
      }
    }
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void MbusCentral::loadPeers() {
  try {
    std::shared_ptr<BaseLib::Database::DataTable> rows = _bl->db->getPeers(_deviceId);
    for (BaseLib::Database::DataTable::iterator row = rows->begin(); row != rows->end(); ++row) {
      int32_t peerID = row->second.at(0)->intValue;
      GD::out.printMessage("Loading M-Bus peer " + std::to_string(peerID));
      std::shared_ptr<MbusPeer> peer(new MbusPeer(peerID, row->second.at(2)->intValue, row->second.at(3)->textValue, _deviceId, this));
      if (!peer->load(this)) continue;
      if (!peer->getRpcDevice()) continue;
      std::lock_guard<std::mutex> peersGuard(_peersMutex);
      if (!peer->getSerialNumber().empty()) _peersBySerial[peer->getSerialNumber()] = peer;
      _peersById[peerID] = peer;
      _peers[peer->getAddress()] = peer;
    }

    std::lock_guard<std::mutex> devicesToPairGuard(_devicesToPairMutex);
    _devicesToPair.clear();
    std::string key = "devicesToPair";
    auto setting = GD::family->getFamilySetting(key);
    if (setting) {
      auto serializedData = setting->binaryValue;
      BaseLib::Rpc::RpcDecoder rpcDecoder(_bl, false, false);
      auto devicesToPair = rpcDecoder.decodeResponse(serializedData);
      for (auto &device: *devicesToPair->arrayValue) {
        if (device->arrayValue->size() != 2 || device->arrayValue->at(0)->integerValue == 0) continue;
        _devicesToPair.emplace(device->arrayValue->at(0)->integerValue, device->arrayValue->at(1)->stringValue);
      }
    }
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

std::shared_ptr<MbusPeer> MbusCentral::getPeer(uint64_t id) {
  try {
    std::lock_guard<std::mutex> peersGuard(_peersMutex);
    if (_peersById.find(id) != _peersById.end()) {
      std::shared_ptr<MbusPeer> peer(std::dynamic_pointer_cast<MbusPeer>(_peersById.at(id)));
      return peer;
    }
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return std::shared_ptr<MbusPeer>();
}

std::shared_ptr<MbusPeer> MbusCentral::getPeer(int32_t address) {
  try {
    std::lock_guard<std::mutex> peersGuard(_peersMutex);
    auto peersIterator = _peers.find(address);
    if (peersIterator != _peers.end()) {
      std::shared_ptr<MbusPeer> peer(std::dynamic_pointer_cast<MbusPeer>(peersIterator->second));
      return peer;
    }
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return std::shared_ptr<MbusPeer>();
}

std::shared_ptr<MbusPeer> MbusCentral::getPeer(std::string serialNumber) {
  try {
    std::lock_guard<std::mutex> peersGuard(_peersMutex);
    if (_peersBySerial.find(serialNumber) != _peersBySerial.end()) {
      std::shared_ptr<MbusPeer> peer(std::dynamic_pointer_cast<MbusPeer>(_peersBySerial.at(serialNumber)));
      return peer;
    }
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return std::shared_ptr<MbusPeer>();
}

bool MbusCentral::onPacketReceived(std::string &senderId, std::shared_ptr<BaseLib::Systems::Packet> packet) {
  try {
    if (_disposing) return false;
    PMbusPacket myPacket(std::dynamic_pointer_cast<MbusPacket>(packet));
    if (!myPacket) return false;

    if (_bl->debugLevel >= 4)
      _bl->out.printInfo(BaseLib::HelperFunctions::getTimeString(myPacket->getTimeReceived()) + " M-Bus packet received (" + senderId + std::string(", RSSI: ") + std::to_string(myPacket->getRssi()) + " dBm" + "): "
                             + BaseLib::HelperFunctions::getHexString(myPacket->getBinary()) + " - Sender address: 0x" + BaseLib::HelperFunctions::getHexString(
          myPacket->secondaryAddress(), 8));

    auto peer = getPeer(myPacket->secondaryAddress());
    if (!peer) {
      if (_sniff) {
        std::lock_guard<std::mutex> sniffedPacketsGuard(_sniffedPacketsMutex);
        auto sniffedPacketsIterator = _sniffedPackets.find(myPacket->secondaryAddress());
        if (sniffedPacketsIterator == _sniffedPackets.end()) {
          _sniffedPackets[myPacket->secondaryAddress()].reserve(100);
          _sniffedPackets[myPacket->secondaryAddress()].push_back(myPacket);
        } else {
          if (sniffedPacketsIterator->second.size() + 1 > sniffedPacketsIterator->second.capacity()) sniffedPacketsIterator->second.reserve(sniffedPacketsIterator->second.capacity() + 100);
          sniffedPacketsIterator->second.push_back(myPacket);
        }
      }

      std::lock_guard<std::mutex> devicesToPairGuard(_devicesToPairMutex);
      auto deviceIterator = _devicesToPair.find(myPacket->secondaryAddress());
      if (deviceIterator != _devicesToPair.end()) {
        std::vector<uint8_t> key = BaseLib::HelperFunctions::getUBinary(deviceIterator->second);
        if (myPacket->getEncryptionMode() != 0 && key.empty()) {
          _bl->out.printInfo("Info: Can't pair device " + BaseLib::HelperFunctions::getHexString(myPacket->secondaryAddress()) + ", because the communication is encrypted and the key is unknown.");
          return false;
        }
        if (!myPacket->decrypt(key) || !myPacket->dataValid()) return false;
        if (myPacket->isEncrypted() && _bl->debugLevel >= 4)
          _bl->out.printInfo(
              BaseLib::HelperFunctions::getTimeString(myPacket->getTimeReceived()) + " Decrypted M-Bus packet: " + BaseLib::HelperFunctions::getHexString(myPacket->getBinary()) + " - Sender address: 0x" + BaseLib::HelperFunctions::getHexString(
                  myPacket->secondaryAddress(), 8));
        pairDevice(myPacket, key);
        peer = getPeer(myPacket->secondaryAddress());
        if (!peer) return false;
      } else if (_pairing) {
        if (myPacket->getEncryptionMode() != 0) {
          _bl->out.printInfo("Info: Can't pair device " + BaseLib::HelperFunctions::getHexString(myPacket->secondaryAddress()) + ", because the communication is encrypted and the key is unknown.");
          return false;
        } else {
          std::vector<uint8_t> key;
          pairDevice(myPacket, key);
          peer = getPeer(myPacket->secondaryAddress());
          if (!peer) return false;
        }
      } else return false;
    }

    if (peer->getEncryptionMode() != myPacket->getEncryptionMode()) {
      _bl->out.printWarning("Warning: Encryption mode of peer " + std::to_string(peer->getID()) + " differs from encryption mode of packet. Dropping it.");
      return false;
    }

    if (myPacket->isEncrypted()) {
      std::vector<uint8_t> aesKey = peer->getAesKey();
      if (!myPacket->decrypt(aesKey) || !myPacket->dataValid()) return false;
      if (_bl->debugLevel >= 4)
        _bl->out.printInfo(
            BaseLib::HelperFunctions::getTimeString(myPacket->getTimeReceived()) + " Decrypted M-Bus packet: " + BaseLib::HelperFunctions::getHexString(myPacket->getBinary()) + " - Sender address: 0x" + BaseLib::HelperFunctions::getHexString(
                myPacket->secondaryAddress(), 8));
      if (_bl->debugLevel >= 5) _bl->out.printDebug("Extended packet info:\n" + myPacket->getInfoString());
    }

    if (peer->getControlInformation() != (int32_t)myPacket->getControlInformation() || peer->getDataRecordCount() != myPacket->dataRecordCount() || (myPacket->isFormatTelegram() && peer->getFormatCrc() != myPacket->getFormatCrc())) {
      if (myPacket->isEncrypted() || senderId == "ExternalInterface") {
        if ((myPacket->isFormatTelegram() || (myPacket->isDataTelegram() && !myPacket->isCompactDataTelegram()))) {
          _bl->out.printInfo(
              "Info: Packet type changed from " + std::to_string(peer->getControlInformation()) + " to " + std::to_string(myPacket->getControlInformation()) + " or data record count changed from " + std::to_string(peer->getDataRecordCount()) + " to "
                  + std::to_string(myPacket->dataRecordCount()) + ". Readding peer " + std::to_string(peer->getID()) + ".");
          std::vector<uint8_t> key = peer->getAesKey();
          peer.reset();

          //Pair again
          pairDevice(myPacket, key);
          peer = getPeer(myPacket->secondaryAddress());
          if (!peer) return false;
        }
      } else {
        _bl->out.printWarning("Warning: Ignoring packet with wrong control information for peer " + std::to_string(peer->getID()) + ". Not changing the peer's configuration as the packet is unencrypted.");
        return false;
      }
    }

    if (!myPacket->isDataTelegram() || myPacket->isFormatTelegram()) return false;

    peer->packetReceived(myPacket);
    return true;
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

void MbusCentral::pairDevice(PMbusPacket packet, std::vector<uint8_t> &key) {
  try {
    if (!packet->isFormatTelegram() && (!packet->isDataTelegram() || packet->isCompactDataTelegram())) return;

    std::lock_guard<std::mutex> pairGuard(_pairMutex);
    GD::out.printInfo("Info: Pairing device 0x" + BaseLib::HelperFunctions::getHexString(packet->secondaryAddress(), 8) + "...");

    bool newPeer = true;
    auto peer = getPeer(packet->secondaryAddress());
    std::unique_lock<std::mutex> lockGuard(_peersMutex);
    if (peer) {
      if (peer->getEncryptionMode() != packet->getEncryptionMode()) {
        _bl->out.printWarning("Warning: Encryption mode of peer " + std::to_string(peer->getID()) + " differs from encryption mode of packet. Not updating peer.");
        return;
      }

      newPeer = false;
      if (_peers.find(peer->getAddress()) != _peers.end()) _peers.erase(peer->getAddress());
      if (_peersBySerial.find(peer->getSerialNumber()) != _peersBySerial.end()) _peersBySerial.erase(peer->getSerialNumber());
      if (_peersById.find(peer->getID()) != _peersById.end()) _peersById.erase(peer->getID());
      lockGuard.unlock();

      int32_t i = 0;
      while (peer.use_count() > 1 && i < 600) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        i++;
      }
      if (i == 600) GD::out.printError("Error: Peer deletion took too long.");
    } else lockGuard.unlock();

    auto peerInfo = _descriptionCreator.createDescription(packet);
    if (peerInfo.serialNumber.empty()) return; //Error
    GD::family->reloadRpcDevices();

    if (!peer) {
      peer = createPeer(peerInfo.type, peerInfo.address, peerInfo.serialNumber, true);
      if (!peer) {
        GD::out.printError("Error: Could not add device with type " + BaseLib::HelperFunctions::getHexString(peerInfo.type) + ". No matching XML file was found.");
        return;
      }
    } else {
      peer->setRpcDevice(GD::family->getRpcDevices()->find(peerInfo.type, 0x10, -1));
      if (!peer->getRpcDevice()) {
        GD::out.printError("Error: RPC device could not be found anymore.");
        return;
      }
    }

    peer->initializeCentralConfig();

    peer->setAesKey(key);
    peer->setControlInformation(packet->getControlInformation());
    peer->setDataRecordCount(packet->dataRecordCount());
    peer->setFormatCrc(packet->getFormatCrc());
    peer->setEncryptionMode(packet->getEncryptionMode());
    peer->setWireless(packet->wireless());
    peer->setPrimaryAddress(packet->primaryAddress());

    lockGuard.lock();
    _peersBySerial[peer->getSerialNumber()] = peer;
    _peersById[peer->getID()] = peer;
    _peers[peer->getAddress()] = peer;
    lockGuard.unlock();

    if (newPeer) {
      GD::out.printInfo("Info: Device successfully added. Peer ID is: " + std::to_string(peer->getID()));

      PVariable deviceDescriptions(new Variable(VariableType::tArray));
      std::shared_ptr<std::vector<PVariable>> descriptions = peer->getDeviceDescriptions(nullptr, true, std::map<std::string, bool>());
      if (!descriptions) return;
      for (std::vector<PVariable>::iterator j = descriptions->begin(); j != descriptions->end(); ++j) {
        deviceDescriptions->arrayValue->push_back(*j);
      }
      std::vector<uint64_t> newIds{peer->getID()};
      raiseRPCNewDevices(newIds, deviceDescriptions);
    } else {
      GD::out.printInfo("Info: Peer " + std::to_string(peer->getID()) + " successfully updated.");

      raiseRPCUpdateDevice(peer->getID(), 0, peer->getSerialNumber() + ":" + std::to_string(0), 0);
    }
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void MbusCentral::savePeers(bool full) {
  try {
    std::lock_guard<std::mutex> peersGuard(_peersMutex);
    for (std::map<uint64_t, std::shared_ptr<BaseLib::Systems::Peer>>::iterator i = _peersById.begin(); i != _peersById.end(); ++i) {
      GD::out.printInfo("Info: Saving M-Bus peer " + std::to_string(i->second->getID()));
      i->second->save(full, full, full);
    }
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void MbusCentral::deletePeer(uint64_t id) {
  try {
    std::shared_ptr<MbusPeer> peer(getPeer(id));
    if (!peer) return;
    peer->deleting = true;
    PVariable deviceAddresses(new Variable(VariableType::tArray));
    deviceAddresses->arrayValue->push_back(PVariable(new Variable(peer->getSerialNumber())));

    PVariable deviceInfo(new Variable(VariableType::tStruct));
    deviceInfo->structValue->insert(StructElement("ID", PVariable(new Variable((int32_t)peer->getID()))));
    PVariable channels(new Variable(VariableType::tArray));
    deviceInfo->structValue->insert(StructElement("CHANNELS", channels));

    for (Functions::iterator i = peer->getRpcDevice()->functions.begin(); i != peer->getRpcDevice()->functions.end(); ++i) {
      deviceAddresses->arrayValue->push_back(PVariable(new Variable(peer->getSerialNumber() + ":" + std::to_string(i->first))));
      channels->arrayValue->push_back(PVariable(new Variable(i->first)));
    }

    std::vector<uint64_t> deletedIds{id};
    raiseRPCDeleteDevices(deletedIds, deviceAddresses, deviceInfo);

    {
      std::lock_guard<std::mutex> peersGuard(_peersMutex);
      if (_peersBySerial.find(peer->getSerialNumber()) != _peersBySerial.end()) _peersBySerial.erase(peer->getSerialNumber());
      if (_peersById.find(id) != _peersById.end()) _peersById.erase(id);
      if (_peers.find(peer->getAddress()) != _peers.end()) _peers.erase(peer->getAddress());
    }

    int32_t i = 0;
    while (peer.use_count() > 1 && i < 600) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      i++;
    }
    if (i == 600) GD::out.printError("Error: Peer deletion took too long.");

    peer->deleteFromDatabase();

    GD::out.printInfo("Info: Deleting XML file \"" + peer->getRpcDevice()->getPath() + "\"");
    GD::bl->io.deleteFile(peer->getRpcDevice()->getPath());

    GD::out.printMessage("Removed M-Bus peer " + std::to_string(peer->getID()));
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

std::string MbusCentral::handleCliCommand(std::string command) {
  try {
    std::ostringstream stringStream;
    std::vector<std::string> arguments;
    bool showHelp = false;
    if (BaseLib::HelperFunctions::checkCliCommand(command, "help", "h", "", 0, arguments, showHelp)) {
      stringStream << "List of commands:" << std::endl << std::endl;
      stringStream << "For more information about the individual command type: COMMAND help" << std::endl << std::endl;
      stringStream << "pairing on (pon)           Enables pairing mode" << std::endl;
      stringStream << "pairing off (pof)          Disables pairing mode" << std::endl;
      stringStream << "peers list (ls)            List all peers" << std::endl;
      stringStream << "peers remove (pr)          Remove a peer" << std::endl;
      stringStream << "peers select (ps)          Select a peer" << std::endl;
      stringStream << "peers setname (pn)         Name a peer" << std::endl;
      stringStream << "unselect (u)               Unselect this device" << std::endl;
      return stringStream.str();
    } else if (BaseLib::HelperFunctions::checkCliCommand(command, "pairing on", "pon", "", 0, arguments, showHelp)) {
      if (showHelp) {
        stringStream << "Description: This command enables pairing mode." << std::endl;
        stringStream << "Usage: pairing on [DURATION]" << std::endl << std::endl;
        stringStream << "Parameters:" << std::endl;
        stringStream << "  DURATION: Optional duration in seconds to stay in pairing mode." << std::endl;
        return stringStream.str();
      }

      int32_t duration = 60;
      if (!arguments.empty()) {
        duration = BaseLib::Math::getNumber(arguments.at(0), false);
        if (duration < 5 || duration > 3600) return "Invalid duration. Duration has to be greater than 5 and less than 3600.\n";
      }

      setInstallMode(nullptr, true, duration, nullptr, false);
      stringStream << "Pairing mode enabled." << std::endl;
      return stringStream.str();
    } else if (BaseLib::HelperFunctions::checkCliCommand(command, "pairing off", "pof", "", 0, arguments, showHelp)) {
      if (showHelp) {
        stringStream << "Description: This command disables pairing mode." << std::endl;
        stringStream << "Usage: pairing off" << std::endl << std::endl;
        stringStream << "Parameters:" << std::endl;
        stringStream << "  There are no parameters." << std::endl;
        return stringStream.str();
      }

      setInstallMode(nullptr, false, -1, nullptr, false);
      stringStream << "Pairing mode disabled." << std::endl;
      return stringStream.str();
    } else if (BaseLib::HelperFunctions::checkCliCommand(command, "peers remove", "pr", "prm", 1, arguments, showHelp)) {
      if (showHelp) {
        stringStream << "Description: This command removes a peer." << std::endl;
        stringStream << "Usage: peers remove PEERID" << std::endl << std::endl;
        stringStream << "Parameters:" << std::endl;
        stringStream << "  PEERID: The id of the peer to remove. Example: 513" << std::endl;
        return stringStream.str();
      }

      uint64_t peerID = BaseLib::Math::getNumber(arguments.at(0), false);
      if (peerID == 0) return "Invalid id.\n";

      if (!peerExists(peerID)) stringStream << "This peer is not paired to this central." << std::endl;
      else {
        stringStream << "Removing peer " << std::to_string(peerID) << std::endl;
        deletePeer(peerID);
      }
      return stringStream.str();
    } else if (BaseLib::HelperFunctions::checkCliCommand(command, "peers list", "pl", "ls", 0, arguments, showHelp)) {
      try {
        if (showHelp) {
          stringStream << "Description: This command lists information about all peers." << std::endl;
          stringStream << "Usage: peers list [FILTERTYPE] [FILTERVALUE]" << std::endl << std::endl;
          stringStream << "Parameters:" << std::endl;
          stringStream << "  FILTERTYPE:  See filter types below." << std::endl;
          stringStream << "  FILTERVALUE: Depends on the filter type. If a number is required, it has to be in hexadecimal format." << std::endl << std::endl;
          stringStream << "Filter types:" << std::endl;
          stringStream << "  ID: Filter by id." << std::endl;
          stringStream << "      FILTERVALUE: The id of the peer to filter (e. g. 513)." << std::endl;
          stringStream << "  SERIAL: Filter by serial number." << std::endl;
          stringStream << "      FILTERVALUE: The serial number of the peer to filter (e. g. JEQ0554309)." << std::endl;
          stringStream << "  ADDRESS: Filter by saddress." << std::endl;
          stringStream << "      FILTERVALUE: The address of the peer to filter (e. g. 128)." << std::endl;
          stringStream << "  NAME: Filter by name." << std::endl;
          stringStream << "      FILTERVALUE: The part of the name to search for (e. g. \"1st floor\")." << std::endl;
          stringStream << "  TYPE: Filter by device type." << std::endl;
          stringStream << "      FILTERVALUE: The 2 byte device type in hexadecimal format." << std::endl;
          return stringStream.str();
        }

        std::string filterType;
        std::string filterValue;

        if (arguments.size() >= 2) {
          filterType = BaseLib::HelperFunctions::toLower(arguments.at(0));
          filterValue = arguments.at(1);
          if (filterType == "name") BaseLib::HelperFunctions::toLower(filterValue);
        }

        if (_peersById.empty()) {
          stringStream << "No peers are paired to this central." << std::endl;
          return stringStream.str();
        }
        std::string bar(" │ ");
        const int32_t idWidth = 8;
        const int32_t nameWidth = 25;
        const int32_t serialWidth = 13;
        const int32_t addressWidth = 8;
        const int32_t typeWidth1 = 8;
        const int32_t typeWidth2 = 45;
        std::string nameHeader("Name");
        nameHeader.resize(nameWidth, ' ');
        std::string typeStringHeader("Type Description");
        typeStringHeader.resize(typeWidth2, ' ');
        stringStream << std::setfill(' ')
                     << std::setw(idWidth) << "ID" << bar
                     << nameHeader << bar
                     << std::setw(serialWidth) << "Serial Number" << bar
                     << std::setw(addressWidth) << "Address" << bar
                     << std::setw(typeWidth1) << "Type" << bar
                     << typeStringHeader
                     << std::endl;
        stringStream << "─────────┼───────────────────────────┼───────────────┼──────────┼──────────┼───────────────────────────────────────────────" << std::endl;
        stringStream << std::setfill(' ')
                     << std::setw(idWidth) << " " << bar
                     << std::setw(nameWidth) << " " << bar
                     << std::setw(serialWidth) << " " << bar
                     << std::setw(addressWidth) << " " << bar
                     << std::setw(typeWidth1) << " " << bar
                     << std::setw(typeWidth2)
                     << std::endl;

        {
          std::lock_guard<std::mutex> peersGuard(_peersMutex);
          for (std::map<uint64_t, std::shared_ptr<BaseLib::Systems::Peer>>::iterator i = _peersById.begin(); i != _peersById.end(); ++i) {
            if (filterType == "id") {
              uint64_t id = BaseLib::Math::getNumber(filterValue, false);
              if (i->second->getID() != id) continue;
            } else if (filterType == "name") {
              std::string name = i->second->getName();
              if ((signed)BaseLib::HelperFunctions::toLower(name).find(filterValue) == (signed)std::string::npos) continue;
            } else if (filterType == "serial") {
              if (i->second->getSerialNumber() != filterValue) continue;
            } else if (filterType == "address") {
              int32_t address = BaseLib::Math::getNumber(filterValue, true);
              if (i->second->getAddress() != address) continue;
            } else if (filterType == "type") {
              int32_t deviceType = BaseLib::Math::getNumber(filterValue, true);
              if ((int32_t)i->second->getDeviceType() != deviceType) continue;
            }

            stringStream << std::setw(idWidth) << std::setfill(' ') << std::to_string(i->second->getID()) << bar;
            std::string name = i->second->getName();
            size_t nameSize = BaseLib::HelperFunctions::utf8StringSize(name);
            if (nameSize > (unsigned)nameWidth) {
              name = BaseLib::HelperFunctions::utf8Substring(name, 0, nameWidth - 3);
              name += "...";
            } else name.resize(nameWidth + (name.size() - nameSize), ' ');
            stringStream << name << bar
                         << std::setw(serialWidth) << i->second->getSerialNumber() << bar
                         << std::setw(addressWidth) << BaseLib::HelperFunctions::getHexString(i->second->getAddress(), 8) << bar
                         << std::setw(typeWidth1) << BaseLib::HelperFunctions::getHexString(i->second->getDeviceType(), 6) << bar;
            if (i->second->getRpcDevice()) {
              PSupportedDevice type = i->second->getRpcDevice()->getType(i->second->getDeviceType(), i->second->getFirmwareVersion());
              std::string typeID;
              if (type) typeID = type->description;
              if (typeID.size() > (unsigned)typeWidth2) {
                typeID.resize(typeWidth2 - 3);
                typeID += "...";
              } else typeID.resize(typeWidth2, ' ');
              stringStream << typeID;
            } else stringStream << std::setw(typeWidth2);
            stringStream << std::endl << std::dec;
          }
        }
        stringStream << "─────────┴───────────────────────────┴───────────────┴──────────┴──────────┴───────────────────────────────────────────────" << std::endl;

        return stringStream.str();
      }
      catch (const std::exception &ex) {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
      }
    } else if (command.compare(0, 13, "peers setname") == 0 || command.compare(0, 2, "pn") == 0) {
      uint64_t peerID = 0;
      std::string name;

      std::stringstream stream(command);
      std::string element;
      int32_t offset = (command.at(1) == 'n') ? 0 : 1;
      int32_t index = 0;
      while (std::getline(stream, element, ' ')) {
        if (index < 1 + offset) {
          index++;
          continue;
        } else if (index == 1 + offset) {
          if (element == "help") break;
          else {
            peerID = BaseLib::Math::getNumber(element, false);
            if (peerID == 0) return "Invalid id.\n";
          }
        } else if (index == 2 + offset) name = element;
        else name += ' ' + element;
        index++;
      }
      if (index == 1 + offset) {
        stringStream << "Description: This command sets or changes the name of a peer to identify it more easily." << std::endl;
        stringStream << "Usage: peers setname PEERID NAME" << std::endl << std::endl;
        stringStream << "Parameters:" << std::endl;
        stringStream << "  PEERID:\tThe id of the peer to set the name for. Example: 513" << std::endl;
        stringStream << "  NAME:\tThe name to set. Example: \"1st floor light switch\"." << std::endl;
        return stringStream.str();
      }

      if (!peerExists(peerID)) stringStream << "This peer is not paired to this central." << std::endl;
      else {
        std::shared_ptr<MbusPeer> peer = getPeer(peerID);
        peer->setName(name);
        stringStream << "Name set to \"" << name << "\"." << std::endl;
      }
      return stringStream.str();
    } else if (BaseLib::HelperFunctions::checkCliCommand(command, "packetunittests", "", "", 0, arguments, showHelp)) {
      //C1 long
      std::vector<uint8_t> data = _bl->hf.getUBinary(
          "FF039C46C5142527706403077225277064C5140007900B00002F2F426C000044130000000001FD171084011300000000C401130000000084021300000000C402130000000084031300000000C403130000000084041300000000C404130000000084051300000000C405130000000084061300000000C406130000000084071300000000C407130000000084081300000000046D1B2F332C04132900000012FF");
      MbusPacket packet(data);
      stringStream << "Parsing packet " << BaseLib::HelperFunctions::getHexString(data) << ":" << std::endl;
      stringStream << packet.getInfoString() << std::endl << std::endl;

      //C1 long compact format
      data = _bl->hf.getUBinary("FF035446C5142527706403076B25277064C5140007970B00002F2F3A2D05426C441301FD17840113C40113840213C40213840313C40313840413C40413840513C40513840613C40613840713C40713840813046D04131229");
      packet = MbusPacket(data);
      stringStream << "Parsing packet " << BaseLib::HelperFunctions::getHexString(data) << ":" << std::endl;
      stringStream << packet.getInfoString() << std::endl << std::endl;

      //C1 long compact data
      data =
          _bl->hf.getUBinary("FF036846C5142527706403077325277064C5140007980B00002F2F2D052549000000000000100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001D2F332C290000001273");
      packet = MbusPacket(data);
      stringStream << "Parsing packet " << BaseLib::HelperFunctions::getHexString(data) << ":" << std::endl;
      stringStream << packet.getInfoString() << std::endl << std::endl;

      //C1 short
      data = _bl->hf.getUBinary("FF032946C5142527706403077225277064C5140007A22B00202F2F046D202F332C04132900000001FD171012E5");
      packet = MbusPacket(data);
      stringStream << "Parsing packet " << BaseLib::HelperFunctions::getHexString(data) << ":" << std::endl;
      stringStream << packet.getInfoString() << std::endl << std::endl;

      //C1 short compact format
      data = _bl->hf.getUBinary("FF032346C5142527706403076B25277064C5140007A92B00202F2F0911F6046D041301FD17123A");
      packet = MbusPacket(data);
      stringStream << "Parsing packet " << BaseLib::HelperFunctions::getHexString(data) << ":" << std::endl;
      stringStream << packet.getInfoString() << std::endl << std::endl;

      //C1 short compact data
      data = _bl->hf.getUBinary("FF032646C5142527706403077325277064C5140007AA2B00202F2F11F605EC232F332C2900000010127B");
      packet = MbusPacket(data);
      stringStream << "Parsing packet " << BaseLib::HelperFunctions::getHexString(data) << ":" << std::endl;
      stringStream << packet.getInfoString() << std::endl << std::endl;

      //C1 short with VIF 7C (not sure, if the packet really looks this way)
      data = _bl->hf.getUBinary("FF033746C5142527706403077225277064C5140007A22B00202F2F046D202F332C04132900000001FD17100D7C0C48656C6C6F20576F726C642112E5");
      packet = MbusPacket(data);
      stringStream << "Parsing packet " << BaseLib::HelperFunctions::getHexString(data) << ":" << std::endl;
      stringStream << packet.getInfoString() << std::endl << std::endl;

      //C1 short format with security mode 7
      data = _bl->hf.getUBinary(
          "FF037246C5143803607403078C0013900F002C250D0000004B5184889B76884D6B38036074C51400071300400710E1E27070E1D6452826C8DD482AA0419872968AB3FB1CD18A91F80CE4F338E78BE8ED2DD0FE29EC20B8479005E378B875D596F85804689FA938582548407F4BB303FD0C02FD0B1250");
      packet = MbusPacket(data);
      stringStream << "Parsing packet " << BaseLib::HelperFunctions::getHexString(data) << ":" << std::endl;
      std::vector<uint8_t> key = _bl->hf.getUBinary("00112233445566778899AABBCCDDEEFF");
      packet.decrypt(key);
      stringStream << packet.getInfoString() << std::endl << std::endl;

      return stringStream.str();
    } else if (BaseLib::HelperFunctions::checkCliCommand(command, "receive", "", "", 1, arguments, showHelp)) {
      if (showHelp) {
        stringStream << "Description: This command simulates the reception of a packet." << std::endl;
        stringStream << "Usage: receive PACKETHEX" << std::endl << std::endl;
        stringStream << "Parameters:" << std::endl;
        stringStream << "  PACKETHEX: The packet to process in hexadecimal format." << std::endl;
        return stringStream.str();
      }

      std::vector<uint8_t> data = _bl->hf.getUBinary(arguments.at(0));
      PMbusPacket packet = std::make_shared<MbusPacket>(data);
      std::string senderId = "TestInterface";
      onPacketReceived(senderId, packet);

      stringStream << "Packet processed. Packet info:" << std::endl << packet->getInfoString() << std::endl;
      return stringStream.str();
    } else if (BaseLib::HelperFunctions::checkCliCommand(command, "crc", "", "", 1, arguments, showHelp)) {
      if (showHelp) {
        stringStream << "Description: This command calculates the CRC for a packet." << std::endl;
        stringStream << "Usage: crc PACKETHEX" << std::endl << std::endl;
        stringStream << "Parameters:" << std::endl;
        stringStream << "  PACKETHEX: The packet to process in hexadecimal format." << std::endl;
        return stringStream.str();
      }

      std::vector<uint8_t> data = BaseLib::HelperFunctions::getUBinary(arguments.at(0));
      Crc16 crc16;
      auto crc = crc16.calculate(data, 0);
      stringStream << BaseLib::HelperFunctions::getHexString(crc, 4) << std::endl;
      return stringStream.str();
    } else return "Unknown command.\n";
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return "Error executing command. See log file for more details.\n";
}

std::shared_ptr<MbusPeer> MbusCentral::createPeer(uint32_t deviceType, int32_t address, std::string serialNumber, bool save) {
  try {
    std::shared_ptr<MbusPeer> peer(new MbusPeer(_deviceId, this));
    peer->setDeviceType(deviceType);
    peer->setAddress(address);
    peer->setSerialNumber(serialNumber);
    peer->setRpcDevice(GD::family->getRpcDevices()->find(deviceType, 0x10, -1));
    if (!peer->getRpcDevice()) return std::shared_ptr<MbusPeer>();
    if (save) peer->save(true, true, false); //Save and create peerID
    return peer;
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return std::shared_ptr<MbusPeer>();
}

PVariable MbusCentral::deleteDevice(BaseLib::PRpcClientInfo clientInfo, std::string serialNumber, int32_t flags) {
  try {
    if (serialNumber.empty()) return Variable::createError(-2, "Unknown device.");

    uint64_t peerId = 0;

    {
      std::shared_ptr<MbusPeer> peer = getPeer(serialNumber);
      if (!peer) return PVariable(new Variable(VariableType::tVoid));
      peerId = peer->getID();
    }

    return deleteDevice(clientInfo, peerId, flags);
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

PVariable MbusCentral::deleteDevice(BaseLib::PRpcClientInfo clientInfo, uint64_t peerId, int32_t flags) {
  try {
    if (peerId == 0) return Variable::createError(-2, "Unknown device.");

    {
      std::shared_ptr<MbusPeer> peer = getPeer(peerId);
      if (!peer) return PVariable(new Variable(VariableType::tVoid));
    }

    deletePeer(peerId);

    if (peerExists(peerId)) return Variable::createError(-1, "Error deleting peer. See log for more details.");

    return PVariable(new Variable(VariableType::tVoid));
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

PVariable MbusCentral::getSniffedDevices(BaseLib::PRpcClientInfo clientInfo) {
  try {
    PVariable array(new Variable(VariableType::tArray));

    std::lock_guard<std::mutex> sniffedPacketsGuard(_sniffedPacketsMutex);
    array->arrayValue->reserve(_sniffedPackets.size());
    for (auto peerPackets: _sniffedPackets) {
      PVariable info(new Variable(VariableType::tStruct));
      array->arrayValue->push_back(info);

      info->structValue->insert(StructElement("FAMILYID", PVariable(new Variable(MY_FAMILY_ID))));
      info->structValue->insert(StructElement("ADDRESS", PVariable(new Variable(peerPackets.first))));
      if (!peerPackets.second.empty()) info->structValue->insert(StructElement("RSSI", PVariable(new Variable(peerPackets.second.back()->getRssi()))));

      PVariable packets(new Variable(VariableType::tArray));
      info->structValue->insert(StructElement("PACKETS", packets));

      for (auto packet: peerPackets.second) {
        PVariable packetInfo(new Variable(VariableType::tStruct));
        packetInfo->structValue->insert(StructElement("TIME_RECEIVED", PVariable(new Variable(packet->getTimeReceived() / 1000))));
        packetInfo->structValue->insert(StructElement("PACKET", PVariable(new Variable(BaseLib::HelperFunctions::getHexString(packet->getBinary())))));
        packets->arrayValue->push_back(packetInfo);
      }
    }
    return array;
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

PVariable MbusCentral::invokeFamilyMethod(BaseLib::PRpcClientInfo clientInfo, std::string &method, PArray parameters) {
  try {
    auto localMethodIterator = _localRpcMethods.find(method);
    if (localMethodIterator != _localRpcMethods.end()) {
      return localMethodIterator->second(clientInfo, parameters);
    } else return BaseLib::Variable::createError(-32601, ": Requested method not found.");
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32502, "Unknown application error.");
}

void MbusCentral::pairingModeTimer(int32_t duration, bool debugOutput) {
  try {
    _pairing = true;
    if (debugOutput) GD::out.printInfo("Info: Pairing mode enabled for " + std::to_string(duration) + " seconds.");
    _timeLeftInPairingMode = duration;
    int64_t startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    int64_t timePassed = 0;
    while (timePassed < ((int64_t)duration * 1000) && !_stopPairingModeThread) {
      std::this_thread::sleep_for(std::chrono::milliseconds(250));
      timePassed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - startTime;
      _timeLeftInPairingMode = duration - (timePassed / 1000);
    }
    _timeLeftInPairingMode = 0;
    _pairing = false;
    if (debugOutput) GD::out.printInfo("Info: Pairing mode disabled.");
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

std::shared_ptr<Variable> MbusCentral::setInstallMode(BaseLib::PRpcClientInfo clientInfo, bool on, uint32_t duration, BaseLib::PVariable metadata, bool debugOutput) {
  try {
    std::lock_guard<std::mutex> pairingModeGuard(_pairingModeThreadMutex);
    if (_disposing) return Variable::createError(-32500, "Central is disposing.");

    /*
     {
       "devices": [
         {
           "address": 64656081,
           "key": "00112233445566778899AABBCCDDEEFF" [optional]
         },
         {
           "address": 64656082,
           "key": "00112233445566778899AABBCCDDEEFF" [optional]
         },
         .
         .
         .
       ]
     }
    */
    std::lock_guard<std::mutex> devicesToPairGuard(_devicesToPairMutex);
    _devicesToPair.clear();
    if (on && metadata) {
      auto devicesIterator = metadata->structValue->find("devices");
      if (devicesIterator != metadata->structValue->end()) {
        for (auto &device: *devicesIterator->second->arrayValue) {
          auto addressIterator = device->structValue->find("address");
          if (addressIterator == device->structValue->end()) continue;
          int32_t address = addressIterator->second->integerValue;
          auto keyIterator = device->structValue->find("key");
          std::string key;
          if (keyIterator != device->structValue->end()) key = keyIterator->second->stringValue;
          _devicesToPair.emplace(address, key);
        }
      }
    }

    BaseLib::PVariable devicesToPair = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tArray);
    devicesToPair->arrayValue->reserve(_devicesToPair.size());
    for (auto &device: _devicesToPair) {
      BaseLib::PVariable element = std::make_shared<BaseLib::Variable>(BaseLib::VariableType::tArray);
      element->arrayValue->reserve(2);
      element->arrayValue->push_back(std::make_shared<BaseLib::Variable>(device.first));
      element->arrayValue->push_back(std::make_shared<BaseLib::Variable>(device.second));
      devicesToPair->arrayValue->push_back(element);
    }

    BaseLib::Rpc::RpcEncoder rpcEncoder(_bl);
    std::vector<char> serializedData;
    rpcEncoder.encodeResponse(devicesToPair, serializedData);
    std::string key = "devicesToPair";
    GD::family->setFamilySetting(key, serializedData);

    _stopPairingModeThread = true;
    _bl->threadManager.join(_pairingModeThread);
    _stopPairingModeThread = false;
    _timeLeftInPairingMode = 0;
    if (on && duration >= 5) {
      _timeLeftInPairingMode = duration; //It's important to set it here, because the thread often doesn't completely initialize before getInstallMode requests _timeLeftInPairingMode
      _bl->threadManager.start(_pairingModeThread, true, &MbusCentral::pairingModeTimer, this, duration, debugOutput);
    }
    return PVariable(new Variable(VariableType::tVoid));
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

PVariable MbusCentral::startSniffing(BaseLib::PRpcClientInfo clientInfo) {
  std::lock_guard<std::mutex> sniffedPacketsGuard(_sniffedPacketsMutex);
  _sniffedPackets.clear();
  _sniff = true;
  return PVariable(new Variable());
}

PVariable MbusCentral::stopSniffing(BaseLib::PRpcClientInfo clientInfo) {
  _sniff = false;
  return PVariable(new Variable());
}

//{{{ Family RPC methods
BaseLib::PVariable MbusCentral::processPacket(const PRpcClientInfo &clientInfo, const PArray &parameters) {
  try {
    if (parameters->empty()) return BaseLib::Variable::createError(-1, "Wrong parameter count.");
    if (parameters->at(0)->type != BaseLib::VariableType::tString) return BaseLib::Variable::createError(-1, "Parameter 1 is not of type String.");

    std::vector<uint8_t> data = _bl->hf.getUBinary(parameters->at(0)->stringValue);
    PMbusPacket packet = std::make_shared<MbusPacket>(data);
    std::string senderId = "ExternalInterface";
    onPacketReceived(senderId, packet);

    return std::make_shared<BaseLib::Variable>(packet->getInfoString());
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}
//}}}

}
