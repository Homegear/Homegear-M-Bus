/* Copyright 2013-2019 Homegear GmbH */

#include "MbusPeer.h"

#include "GD.h"
#include "MbusPacket.h"
#include "MbusCentral.h"

#include <iomanip>

namespace Mbus {
std::shared_ptr<BaseLib::Systems::ICentral> MbusPeer::getCentral() {
  try {
    if (_central) return _central;
    _central = GD::family->getCentral();
    return _central;
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return std::shared_ptr<BaseLib::Systems::ICentral>();
}

MbusPeer::MbusPeer(uint32_t parentID, IPeerEventSink *eventHandler) : BaseLib::Systems::Peer(GD::bl, parentID, eventHandler) {
  init();
}

MbusPeer::MbusPeer(int32_t id, int32_t address, std::string serialNumber, uint32_t parentID, IPeerEventSink *eventHandler) : BaseLib::Systems::Peer(GD::bl, id, address, serialNumber, parentID, eventHandler) {
  init();
}

MbusPeer::~MbusPeer() {
  try {
    dispose();
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void MbusPeer::init() {
  try {
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void MbusPeer::dispose() {
  if (_disposing) return;
  Peer::dispose();
}

void MbusPeer::worker() {
  try {
    if (!serviceMessages->getUnreach()) serviceMessages->checkUnreach(_rpcDevice->timeout, getLastPacketReceived());
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void MbusPeer::homegearStarted() {
  try {
    Peer::homegearStarted();
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void MbusPeer::homegearShuttingDown() {
  try {
    _shuttingDown = true;
    Peer::homegearShuttingDown();
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

std::string MbusPeer::handleCliCommand(std::string command) {
  try {
    std::ostringstream stringStream;

    if (command == "help") {
      stringStream << "List of commands:" << std::endl << std::endl;
      stringStream << "For more information about the individual command type: COMMAND help" << std::endl << std::endl;
      stringStream << "unselect\t\tUnselect this peer" << std::endl;
      stringStream << "channel count\t\tPrint the number of channels of this peer" << std::endl;
      stringStream << "config print\t\tPrints all configuration parameters and their values" << std::endl;
      return stringStream.str();
    }
    if (command.compare(0, 13, "channel count") == 0) {
      std::stringstream stream(command);
      std::string element;
      int32_t index = 0;
      while (std::getline(stream, element, ' ')) {
        if (index < 2) {
          index++;
          continue;
        } else if (index == 2) {
          if (element == "help") {
            stringStream << "Description: This command prints this peer's number of channels." << std::endl;
            stringStream << "Usage: channel count" << std::endl << std::endl;
            stringStream << "Parameters:" << std::endl;
            stringStream << "  There are no parameters." << std::endl;
            return stringStream.str();
          }
        }
        index++;
      }

      stringStream << "Peer has " << _rpcDevice->functions.size() << " channels." << std::endl;
      return stringStream.str();
    } else if (command.compare(0, 12, "config print") == 0) {
      std::stringstream stream(command);
      std::string element;
      int32_t index = 0;
      while (std::getline(stream, element, ' ')) {
        if (index < 2) {
          index++;
          continue;
        } else if (index == 2) {
          if (element == "help") {
            stringStream << "Description: This command prints all configuration parameters of this peer. The values are in BidCoS packet format." << std::endl;
            stringStream << "Usage: config print" << std::endl << std::endl;
            stringStream << "Parameters:" << std::endl;
            stringStream << "  There are no parameters." << std::endl;
            return stringStream.str();
          }
        }
        index++;
      }

      return printConfig();
    } else return "Unknown command.\n";
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return "Error executing command. See log file for more details.\n";
}

std::string MbusPeer::printConfig() {
  try {
    std::ostringstream stringStream;
    stringStream << "MASTER" << std::endl;
    stringStream << "{" << std::endl;
    for (std::unordered_map<uint32_t, std::unordered_map<std::string, BaseLib::Systems::RpcConfigurationParameter>>::iterator i = configCentral.begin(); i != configCentral.end(); ++i) {
      stringStream << "\t" << "Channel: " << std::dec << i->first << std::endl;
      stringStream << "\t{" << std::endl;
      for (std::unordered_map<std::string, BaseLib::Systems::RpcConfigurationParameter>::iterator j = i->second.begin(); j != i->second.end(); ++j) {
        stringStream << "\t\t[" << j->first << "]: ";
        if (!j->second.rpcParameter) stringStream << "(No RPC parameter) ";
        std::vector<uint8_t> parameterData = j->second.getBinaryData();
        for (std::vector<uint8_t>::const_iterator k = parameterData.begin(); k != parameterData.end(); ++k) {
          stringStream << std::hex << std::setfill('0') << std::setw(2) << (int32_t)*k << " ";
        }
        stringStream << std::endl;
      }
      stringStream << "\t}" << std::endl;
    }
    stringStream << "}" << std::endl << std::endl;

    stringStream << "VALUES" << std::endl;
    stringStream << "{" << std::endl;
    for (std::unordered_map<uint32_t, std::unordered_map<std::string, BaseLib::Systems::RpcConfigurationParameter>>::iterator i = valuesCentral.begin(); i != valuesCentral.end(); ++i) {
      stringStream << "\t" << "Channel: " << std::dec << i->first << std::endl;
      stringStream << "\t{" << std::endl;
      for (std::unordered_map<std::string, BaseLib::Systems::RpcConfigurationParameter>::iterator j = i->second.begin(); j != i->second.end(); ++j) {
        stringStream << "\t\t[" << j->first << "]: ";
        if (!j->second.rpcParameter) stringStream << "(No RPC parameter) ";
        std::vector<uint8_t> parameterData = j->second.getBinaryData();
        for (std::vector<uint8_t>::const_iterator k = parameterData.begin(); k != parameterData.end(); ++k) {
          stringStream << std::hex << std::setfill('0') << std::setw(2) << (int32_t)*k << " ";
        }
        stringStream << std::endl;
      }
      stringStream << "\t}" << std::endl;
    }
    stringStream << "}" << std::endl << std::endl;

    return stringStream.str();
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return "";
}

void MbusPeer::loadVariables(BaseLib::Systems::ICentral *central, std::shared_ptr<BaseLib::Database::DataTable> &rows) {
  try {
    if (!rows) rows = _bl->db->getPeerVariables(_peerID);
    Peer::loadVariables(central, rows);

    _rpcDevice = GD::family->getRpcDevices()->find(_deviceType, _firmwareVersion, -1);
    if (!_rpcDevice) return;

    for (BaseLib::Database::DataTable::iterator row = rows->begin(); row != rows->end(); ++row) {
      switch (row->second.at(2)->intValue) {
        case 21: {
          _aesKey.clear();
          _aesKey.insert(_aesKey.end(), row->second.at(5)->binaryValue->begin(), row->second.at(5)->binaryValue->end());
          break;
        }
        case 22: {
          _controlInformation = row->second.at(3)->intValue;
          break;
        }
        case 23: {
          _dataRecordCount = row->second.at(3)->intValue;
          break;
        }
        case 24: {
          _formatCrc = row->second.at(3)->intValue;
          break;
        }
        case 25: {
          _encryptionMode = row->second.at(3)->intValue;
          break;
        }
        case 26: {
          _lastTime = row->second.at(3)->intValue;
          break;
        }
        case 27: {
          _wireless = (bool)row->second.at(3)->intValue;
          break;
        }
        case 28: {
          _primaryAddress = row->second.at(3)->intValue;
          break;
        }
      }
    }
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void MbusPeer::saveVariables() {
  try {
    if (_peerID == 0) return;
    Peer::saveVariables();
    saveVariable(21, _aesKey);
    saveVariable(22, _controlInformation);
    saveVariable(23, _dataRecordCount);
    saveVariable(24, _formatCrc);
    saveVariable(25, _encryptionMode);
    saveVariable(26, _lastTime);
    saveVariable(27, (int32_t)_wireless);
    saveVariable(28, _primaryAddress);
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

bool MbusPeer::load(BaseLib::Systems::ICentral *central) {
  try {
    std::shared_ptr<BaseLib::Database::DataTable> rows;
    loadVariables(central, rows);
    if (!_rpcDevice) {
      GD::out.printError("Error loading peer " + std::to_string(_peerID) + ": Device type not found: 0x" + BaseLib::HelperFunctions::getHexString(_deviceType) + " Firmware version: " + std::to_string(_firmwareVersion));
      return false;
    }

    initializeTypeString();
    std::string entry;
    loadConfig();
    initializeCentralConfig();

    serviceMessages.reset(new BaseLib::Systems::ServiceMessages(_bl, _peerID, _serialNumber, this));
    serviceMessages->load();

    return true;
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

void MbusPeer::setRssiDevice(uint8_t rssi) {
  try {
    if (_disposing || rssi == 0) return;
    uint32_t time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (time - _lastRssiDevice > 10) {
      _lastRssiDevice = time;

      std::unordered_map<uint32_t, std::unordered_map<std::string, BaseLib::Systems::RpcConfigurationParameter>>::iterator channelIterator = valuesCentral.find(0);
      if (channelIterator == valuesCentral.end()) return;
      std::unordered_map<std::string, BaseLib::Systems::RpcConfigurationParameter>::iterator parameterIterator = channelIterator->second.find("RSSI_DEVICE");
      if (parameterIterator == channelIterator->second.end()) return;

      BaseLib::Systems::RpcConfigurationParameter &parameter = parameterIterator->second;
      std::vector<uint8_t> parameterData{rssi};
      parameter.setBinaryData(parameterData);

      std::shared_ptr<std::vector<std::string>> valueKeys(new std::vector<std::string>({std::string("RSSI_DEVICE")}));
      std::shared_ptr<std::vector<PVariable>> rpcValues(new std::vector<PVariable>());
      rpcValues->push_back(parameter.rpcParameter->convertFromPacket(parameterData, parameter.mainRole(), false));

      std::string eventSource = "device-" + std::to_string(_peerID);
      std::string address = _serialNumber + ":0";
      raiseEvent(eventSource, _peerID, 0, valueKeys, rpcValues);
      raiseRPCEvent(eventSource, _peerID, 0, address, valueKeys, rpcValues);
    }
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void MbusPeer::getValuesFromPacket(PMbusPacket packet, std::vector<FrameValues> &frameValues) {
  try {
    if (!_rpcDevice) return;

    //Check for low battery
    if (packet->batteryEmpty()) {
      serviceMessages->set("LOWBAT", true);
      if (_bl->debugLevel >= 4) GD::out.printInfo("Info: LOWBAT of peer " + std::to_string(_peerID) + " with serial number " + _serialNumber + " was set to \"true\".");
    } else serviceMessages->set("LOWBAT", false);

    //equal_range returns all elements with "0" or an unknown element as argument
    if (_rpcDevice->packetsByMessageType.find(1) == _rpcDevice->packetsByMessageType.end()) return;
    std::pair<PacketsByMessageType::iterator, PacketsByMessageType::iterator> range = _rpcDevice->packetsByMessageType.equal_range(1);
    if (range.first == _rpcDevice->packetsByMessageType.end()) return;
    PacketsByMessageType::iterator i = range.first;
    do {
      FrameValues currentFrameValues;
      PPacket frame(i->second);
      if (!frame) continue;
      std::vector<uint8_t> payload = packet->getPayload();
      if (payload.empty()) break;
      uint32_t payloadBitSize = payload.size() * 8;
      int32_t channelIndex = frame->channelIndex;
      int32_t channel = -1;
      if (channelIndex >= 0 && channelIndex < (signed)payload.size()) channel = payload.at(channelIndex);
      if (channel > -1 && frame->channelSize < 8.0) channel &= (0xFFu >> (8u - std::lround(frame->channelSize)));
      channel += frame->channelIndexOffset;
      if (frame->channel > -1) channel = frame->channel;
      if (channel == -1) continue;
      currentFrameValues.frameID = frame->id;
      bool abort = false;

      for (BinaryPayloads::iterator j = frame->binaryPayloads.begin(); j != frame->binaryPayloads.end(); ++j) {
        std::vector<uint8_t> data;
        if ((*j)->bitSize > 0 && (*j)->bitIndex > 0) {
          if ((*j)->bitIndex >= payloadBitSize) continue;
          data = packet->getPosition((*j)->bitIndex, (*j)->bitSize);

          if ((*j)->constValueInteger > -1) {
            int32_t intValue = 0;
            BaseLib::HelperFunctions::memcpyBigEndian(intValue, data);
            if (intValue != (*j)->constValueInteger) {
              abort = true;
              break;
            } else if ((*j)->parameterId.empty()) continue;
          }
        } else if ((*j)->constValueInteger > -1) {
          BaseLib::HelperFunctions::memcpyBigEndian(data, (*j)->constValueInteger);
        } else continue;

        for (std::vector<PParameter>::iterator k = frame->associatedVariables.begin(); k != frame->associatedVariables.end(); ++k) {
          if ((*k)->physical->groupId != (*j)->parameterId) continue;
          currentFrameValues.parameterSetType = (*k)->parent()->type();
          bool setValues = false;
          if (currentFrameValues.paramsetChannels.empty()) //Fill paramsetChannels
          {
            int32_t startChannel = (channel < 0) ? 0 : channel;
            int32_t endChannel;
            //When fixedChannel is -2 (means '*') cycle through all channels
            if (frame->channel == -2) {
              startChannel = 0;
              endChannel = _rpcDevice->functions.rbegin()->first;
            } else endChannel = startChannel;
            for (int32_t l = startChannel; l <= endChannel; l++) {
              Functions::iterator functionIterator = _rpcDevice->functions.find(l);
              if (functionIterator == _rpcDevice->functions.end()) continue;
              PParameterGroup parameterGroup = functionIterator->second->getParameterGroup(currentFrameValues.parameterSetType);
              if (!parameterGroup || parameterGroup->parameters.find((*k)->id) == parameterGroup->parameters.end()) continue;
              currentFrameValues.paramsetChannels.push_back(l);
              currentFrameValues.values[(*k)->id].channels.push_back(l);
              setValues = true;
            }
          } else //Use paramsetChannels
          {
            for (std::list<uint32_t>::const_iterator l = currentFrameValues.paramsetChannels.begin(); l != currentFrameValues.paramsetChannels.end(); ++l) {
              Functions::iterator functionIterator = _rpcDevice->functions.find(*l);
              if (functionIterator == _rpcDevice->functions.end()) continue;
              PParameterGroup parameterGroup = functionIterator->second->getParameterGroup(currentFrameValues.parameterSetType);
              if (!parameterGroup || parameterGroup->parameters.find((*k)->id) == parameterGroup->parameters.end()) continue;
              currentFrameValues.values[(*k)->id].channels.push_back(*l);
              setValues = true;
            }
          }
          if (setValues) currentFrameValues.values[(*k)->id].value = data;
        }
      }
      if (abort) continue;
      if (!currentFrameValues.values.empty()) frameValues.push_back(currentFrameValues);
    } while (++i != range.second && i != _rpcDevice->packetsByMessageType.end());
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void MbusPeer::packetReceived(PMbusPacket &packet) {
  try {
    if (_disposing || !packet || !_rpcDevice) return;
    if (packet->secondaryAddress() != _address) return;
    if (_formatCrc != 0 && packet->getFormatCrc() != _formatCrc) {
      GD::out.printWarning("Warning: Ignoring packet with wrong format frame CRC.");
      return;
    }
    if (getEncryptionMode() != packet->getEncryptionMode()) {
      GD::out.printWarning("Warning: Ignoring packet with wrong encryption mode.");
      return;
    }
    std::shared_ptr<MbusCentral> central = std::dynamic_pointer_cast<MbusCentral>(getCentral());
    if (!central) return;
    setLastPacketReceived();
    setRssiDevice(packet->getRssi() * -1);
    serviceMessages->endUnreach();

    std::vector<FrameValues> frameValues;
    getValuesFromPacket(packet, frameValues);
    std::map<uint32_t, std::shared_ptr<std::vector<std::string>>> valueKeys;
    std::map<uint32_t, std::shared_ptr<std::vector<PVariable>>> rpcValues;

    //Loop through all matching frames
    for (std::vector<FrameValues>::iterator a = frameValues.begin(); a != frameValues.end(); ++a) {
      PPacket frame;
      if (!a->frameID.empty()) frame = _rpcDevice->packetsById.at(a->frameID);
      if (!frame) continue;

      for (std::map<std::string, FrameValue>::iterator i = a->values.begin(); i != a->values.end(); ++i) {
        for (std::list<uint32_t>::const_iterator j = a->paramsetChannels.begin(); j != a->paramsetChannels.end(); ++j) {
          if (std::find(i->second.channels.begin(), i->second.channels.end(), *j) == i->second.channels.end()) continue;
          if (!valueKeys[*j] || !rpcValues[*j]) {
            valueKeys[*j].reset(new std::vector<std::string>());
            rpcValues[*j].reset(new std::vector<PVariable>());
          }

          BaseLib::Systems::RpcConfigurationParameter &parameter = valuesCentral[*j][i->first];
          if (parameter.equals(i->second.value)) continue;
          parameter.setBinaryData(i->second.value);
          if (parameter.databaseId > 0) saveParameter(parameter.databaseId, i->second.value);
          else saveParameter(0, ParameterGroup::Type::Enum::variables, *j, i->first, i->second.value);
          if (_bl->debugLevel >= 4)
            GD::out.printInfo(
                "Info: " + i->first + " on channel " + std::to_string(*j) + " of peer " + std::to_string(_peerID) + " with serial number " + _serialNumber + " was set to 0x" + BaseLib::HelperFunctions::getHexString(i->second.value) + ".");

          if (parameter.rpcParameter) {
            if (parameter.rpcParameter->casts.empty()) continue;
            ParameterCast::PGeneric parameterCast = std::dynamic_pointer_cast<ParameterCast::Generic>(parameter.rpcParameter->casts.at(0));
            if (!parameterCast) continue;

            uint8_t type = BaseLib::Math::getUnsignedNumber(parameterCast->type);
            std::vector<uint8_t> vifs = _bl->hf.getUBinary(parameter.rpcParameter->metadata);

            //Process service messages
            if (parameter.rpcParameter->service && !i->second.value.empty()) {
              if (parameter.rpcParameter->logical->type == ILogical::Type::Enum::tEnum) {
                serviceMessages->set(i->first, i->second.value.at(0), *j);
              } else if (parameter.rpcParameter->logical->type == ILogical::Type::Enum::tBoolean) {
                serviceMessages->set(i->first, VifConverter::getVariable(type, vifs, i->second.value)->booleanValue);
              }
            }

            BaseLib::PVariable value = VifConverter::getVariable(type, vifs, i->second.value);

            if (i->first == "DATE" || i->first == "DATETIME") {
              int32_t time = BaseLib::HelperFunctions::getTimeSeconds();
              if (packet->wireless() && (value->integerValue < time - (86400 * 2) || value->integerValue > time + (86400 * 2))) {
                serviceMessages->set("POSSIBLE_HACKING_ATTEMPT", true);
                GD::out.printWarning("Warning: Possible hacking attempt. Date in packet deviates more than two days from current date.");
              } else if (packet->wireless() && value->integerValue < _lastTime) {
                serviceMessages->set("POSSIBLE_HACKING_ATTEMPT", true);
                GD::out.printWarning("Warning: Possible hacking attempt. Date in packet is older than in last packet.");
              } else _lastTime = value->integerValue;
            }

            valueKeys[*j]->push_back(i->first);
            rpcValues[*j]->push_back(value);
          }
        }
      }
    }

    if (!rpcValues.empty()) {
      for (auto &valueKey: valueKeys) {
        if (valueKey.second->empty()) continue;
        std::string eventSource = "device-" + std::to_string(_peerID);
        std::string address(_serialNumber + ":" + std::to_string(valueKey.first));
        raiseEvent(eventSource, _peerID, valueKey.first, valueKey.second, rpcValues.at(valueKey.first));
        raiseRPCEvent(eventSource, _peerID, valueKey.first, address, valueKey.second, rpcValues.at(valueKey.first));
      }
    }
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

PParameterGroup MbusPeer::getParameterSet(int32_t channel, ParameterGroup::Type::Enum type) {
  try {
    PFunction rpcChannel = _rpcDevice->functions.at(channel);
    if (type == ParameterGroup::Type::Enum::variables) return rpcChannel->variables;
    else if (type == ParameterGroup::Type::Enum::config) return rpcChannel->configParameters;
    else if (type == ParameterGroup::Type::Enum::link) return rpcChannel->linkParameters;
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return PParameterGroup();
}

bool MbusPeer::getAllValuesHook2(PRpcClientInfo clientInfo, PParameter parameter, uint32_t channel, PVariable parameters) {
  try {
    if (channel == 1) {
      if (parameter->id == "PEER_ID") {
        std::vector<uint8_t> parameterData;
        auto &rpcConfigurationParameter = valuesCentral[channel][parameter->id];
        parameter->convertToPacket(PVariable(new Variable((int32_t)_peerID)), rpcConfigurationParameter.mainRole(), parameterData);
        rpcConfigurationParameter.setBinaryData(parameterData);
      }
    }
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

bool MbusPeer::getParamsetHook2(PRpcClientInfo clientInfo, PParameter parameter, uint32_t channel, PVariable parameters) {
  try {
    if (channel == 1) {
      if (parameter->id == "PEER_ID") {
        std::vector<uint8_t> parameterData;
        auto &rpcConfigurationParameter = valuesCentral[channel][parameter->id];
        parameter->convertToPacket(PVariable(new Variable((int32_t)_peerID)), rpcConfigurationParameter.mainRole(), parameterData);
        rpcConfigurationParameter.setBinaryData(parameterData);
      }
    }
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

bool MbusPeer::convertFromPacketHook(BaseLib::Systems::RpcConfigurationParameter &parameter, std::vector<uint8_t> &data, PVariable &result) {
  try {
    if (!parameter.rpcParameter) return false;
    if (parameter.rpcParameter->casts.empty()) return false;
    ParameterCast::PGeneric cast = std::dynamic_pointer_cast<ParameterCast::Generic>(parameter.rpcParameter->casts.at(0));
    if (!cast) return false;

    uint8_t type = BaseLib::Math::getUnsignedNumber(cast->type);
    std::vector<uint8_t> vifs = BaseLib::HelperFunctions::getUBinary(parameter.rpcParameter->metadata);

    result = VifConverter::getVariable(type, vifs, data);
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return true;
}

PVariable MbusPeer::putParamset(BaseLib::PRpcClientInfo clientInfo, int32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteID, int32_t remoteChannel, PVariable variables, bool checkAcls, bool onlyPushing) {
  try {
    if (_disposing) return Variable::createError(-32500, "Peer is disposing.");
    if (channel < 0) channel = 0;
    if (remoteChannel < 0) remoteChannel = 0;
    Functions::iterator functionIterator = _rpcDevice->functions.find(channel);
    if (functionIterator == _rpcDevice->functions.end()) return Variable::createError(-2, "Unknown channel.");
    if (type == ParameterGroup::Type::none) type = ParameterGroup::Type::link;
    PParameterGroup parameterGroup = functionIterator->second->getParameterGroup(type);
    if (!parameterGroup) return Variable::createError(-3, "Unknown parameter set.");
    if (variables->structValue->empty()) return PVariable(new Variable(VariableType::tVoid));

    auto central = getCentral();
    if (!central) return Variable::createError(-32500, "Could not get central.");

    if (type == ParameterGroup::Type::Enum::config) {
      bool parameterChanged = false;
      for (Struct::iterator i = variables->structValue->begin(); i != variables->structValue->end(); ++i) {
        if (i->first.empty() || !i->second) continue;
        if (configCentral[channel].find(i->first) == configCentral[channel].end()) continue;
        BaseLib::Systems::RpcConfigurationParameter &parameter = configCentral[channel][i->first];
        if (!parameter.rpcParameter) continue;
        if (parameter.rpcParameter->password && i->second->stringValue.empty()) continue; //Don't safe password if empty
        std::vector<uint8_t> parameterData;
        parameter.rpcParameter->convertToPacket(i->second, parameter.mainRole(), parameterData);
        parameter.setBinaryData(parameterData);
        if (parameter.databaseId > 0) saveParameter(parameter.databaseId, parameterData);
        else saveParameter(0, ParameterGroup::Type::Enum::config, channel, i->first, parameterData);
        parameterChanged = true;
        GD::out.printInfo("Info: Parameter " + i->first + " of peer " + std::to_string(_peerID) + " and channel " + std::to_string(channel) + " was set to 0x" + BaseLib::HelperFunctions::getHexString(parameterData) + ".");
      }

      if (parameterChanged) raiseRPCUpdateDevice(_peerID, channel, _serialNumber + ":" + std::to_string(channel), 0);
    } else if (type == ParameterGroup::Type::Enum::variables) {
      for (Struct::iterator i = variables->structValue->begin(); i != variables->structValue->end(); ++i) {
        if (i->first.empty() || !i->second) continue;

        if (checkAcls && !clientInfo->acls->checkVariableWriteAccess(central->getPeer(_peerID), channel, i->first)) continue;

        setValue(clientInfo, channel, i->first, i->second, false);
      }
    } else {
      return Variable::createError(-3, "Parameter set type is not supported.");
    }
    return std::make_shared<Variable>(VariableType::tVoid);
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error.");
}

PVariable MbusPeer::setValue(BaseLib::PRpcClientInfo clientInfo, uint32_t channel, std::string valueKey, PVariable value, bool wait) {
  try {
    Peer::setValue(clientInfo, channel, valueKey, value, wait); //Ignore result, otherwise setHomegerValue might not be executed
    if (_disposing) return Variable::createError(-32500, "Peer is disposing.");
    if (valueKey.empty()) return Variable::createError(-5, "Value key is empty.");
    if (channel == 0 && serviceMessages->set(valueKey, value->booleanValue)) return std::make_shared<BaseLib::Variable>();
    return Variable::createError(-5, "Unknown parameter.");
  }
  catch (const std::exception &ex) {
    GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return Variable::createError(-32500, "Unknown application error. See error log for more details.");
}

}
