/* Copyright 2013-2019 Homegear GmbH */

#include "MbusPacket.h"

#include "Gd.h"

namespace Mbus {
MbusPacket::MbusPacket() {
  _difSizeMap[0] = 0; //No data
  _difSizeMap[1] = 1; //8 bit integer
  _difSizeMap[2] = 2; //16 bit integer
  _difSizeMap[3] = 3; //24 bit integer
  _difSizeMap[4] = 4; //32 bit integer
  _difSizeMap[5] = 4; //32 bit floating point
  _difSizeMap[6] = 6; //48 bit integer
  _difSizeMap[7] = 8; //64 bit integer
  _difSizeMap[8] = 0; //"Auswahl für Ablesung"
  _difSizeMap[9] = 1; //2 digit BCD
  _difSizeMap[10] = 2; //4 digit BCD
  _difSizeMap[11] = 3; //6 digit BCD
  _difSizeMap[12] = 4; //8 digit BCD
  _difSizeMap[13] = 0; //Variable length
  _difSizeMap[14] = 6; //12 digit BCD
  _difSizeMap[15] = 0; //Special functions
}

MbusPacket::MbusPacket(const std::vector<uint8_t> &packet) : MbusPacket() {
  _packet = packet;
  _timeReceived = BaseLib::HelperFunctions::getTime();

  size_t ciStart = packet.size(); //Set out of range
  if (packet.at(0) == 0xFF) {
    _wireless = true;
    _rssi = packet.at(packet.size() - 2);
    if (_rssi >= 128) _rssi = ((_rssi - 256) / 2) - 74; //From Amber wireless datasheet and CC1101/CC110L datasheet
    else _rssi = (_rssi / 2) - 74;

    _command = packet.at(1);
    _length = packet.at(2);
    _control = packet.at(3);
    _manufacturerCode = (((uint32_t)packet.at(5)) << 8u) | packet.at(4);
    _manufacturer.clear();
    _manufacturer.reserve(3);
    _manufacturer.push_back((char)(((_manufacturerCode >> 10u) & 0x1Fu) + 64));
    _manufacturer.push_back((char)(((_manufacturerCode >> 5u) & 0x1Fu) + 64));
    _manufacturer.push_back((char)((_manufacturerCode & 0x1Fu) + 64));
    _iv.clear();
    _iv.reserve(16);
    _iv.insert(_iv.end(), packet.begin() + 4, packet.begin() + 12);
    _secondaryAddress = (((uint32_t)packet.at(9)) << 24u) | (((uint32_t)packet.at(8)) << 16u) | (((uint32_t)packet.at(7)) << 8u) | ((uint32_t)packet.at(6));
    _version = packet.at(10);
    _medium = packet.at(11);
    ciStart = 12;
  } else if (packet.at(0) == 0x68) {
    _wireless = false;
    if (packet.at(0) == 0x68 && packet.at(3) == 0x68) {
      _length = packet.at(1);
      _control = packet.at(4);
      _primaryAddress = packet.at(5);
      ciStart = 6;
    }
  } else {
    Gd::out.printWarning("Warning: Unknown packet type: " + BaseLib::HelperFunctions::getHexString(packet.at(0)));
    return;
  }

  uint8_t controlInformation = 0;
  for (int32_t i = 0; i < 10; i++) {
    if (ciStart >= packet.size()) break;
    controlInformation = packet.at(ciStart);
    if (controlInformation == 0x8C) //ELL I
    {
      //This value of the CI-field is used if data encryption at the link layer is not used in the frame.
      _ellInfo.controlInformation = controlInformation;
      _ellInfo.communicationControlField.raw = _packet.at(ciStart + 1);
      _ellInfo.accessNumber = packet.at(ciStart + 2);
      ciStart += 3;
      continue;
    } else if (controlInformation == 0x8D) //ELL II
    {
      //This value of the CI-field is used if data encryption at the link layer is used in the frame.
      _ellInfo.controlInformation = controlInformation;
      _ellInfo.communicationControlField.raw = _packet.at(ciStart + 1);
      _ellInfo.accessNumber = packet.at(ciStart + 2);
      _ellInfo.sessionNumberField.raw = ((uint32_t)packet.at(ciStart + 6) << 24u) | ((uint32_t)packet.at(ciStart + 5) << 16u) | ((uint32_t)packet.at(ciStart + 4) << 8u) | packet.at(ciStart + 3);
      _ellInfo.ellEncryption = _ellInfo.sessionNumberField.bitField.encryptionField == 1;
      if (_ellInfo.ellEncryption) {
        _encryptionMode = 1;
        _iv.emplace_back(_ellInfo.communicationControlField.raw);
        _iv.insert(_iv.end(), packet.begin() + ciStart + 3, packet.begin() + ciStart + 7);
        _iv.emplace_back(0); //Frame number - Only needed for bidirectional communication
        _iv.emplace_back(0); //Frame number - Only needed for bidirectional communication - increments with each packet within a session
        _iv.emplace_back(0); //Block counter - Needs to be incremented for each block
        _payload.clear();
        _payload.insert(_payload.end(), packet.begin() + ciStart + 7, packet.end() - 2);
        break;
      }
      ciStart += 9;
      continue;
    } else if (controlInformation == 0x8E) //ELL III
    {
      //This value of the CI-field is used if data encryption at the link layer is not used in the frame. This
      //extended link layer specifies the receiver address.
      _ellInfo.controlInformation = controlInformation;
      _ellInfo.communicationControlField.raw = _packet.at(ciStart + 1);
      _ellInfo.accessNumber = packet.at(ciStart + 2);
      uint32_t value = (((uint32_t)packet.at(ciStart + 4)) << 8u) | packet.at(ciStart + 3);
      _ellInfo.manufacturer2.clear();
      _ellInfo.manufacturer2.reserve(3);
      _ellInfo.manufacturer2.push_back((char)(((value >> 10u) & 0x1Fu) + 64));
      _ellInfo.manufacturer2.push_back((char)(((value >> 5u) & 0x1Fu) + 64));
      _ellInfo.manufacturer2.push_back((char)((value & 0x1Fu) + 64));
      _ellInfo.address2 = (((uint32_t)packet.at(ciStart + 8)) << 24u) | (((uint32_t)packet.at(ciStart + 7)) << 16u) | (((uint32_t)packet.at(ciStart + 6)) << 8u) | ((uint32_t)packet.at(ciStart + 5));
      _ellInfo.version2 = packet.at(ciStart + 9);
      _ellInfo.medium2 = packet.at(ciStart + 10);
      ciStart += 11;
      continue;
    } else if (controlInformation == 0x8F) //ELL IV
    {
      //This value of the CI-field is used if data encryption at the link layer is used in the frame. This extended
      //link layer specifies the receiver address.
      _ellInfo.controlInformation = controlInformation;
      _ellInfo.communicationControlField.raw = _packet.at(ciStart + 1);
      _ellInfo.accessNumber = packet.at(ciStart + 2);
      uint32_t value = (((uint32_t)packet.at(ciStart + 4)) << 8u) | packet.at(ciStart + 3);
      _ellInfo.manufacturer2.clear();
      _ellInfo.manufacturer2.reserve(3);
      _ellInfo.manufacturer2.push_back((char)(((value >> 10u) & 0x1Fu) + 64));
      _ellInfo.manufacturer2.push_back((char)(((value >> 5u) & 0x1Fu) + 64));
      _ellInfo.manufacturer2.push_back((char)((value & 0x1Fu) + 64));
      _ellInfo.address2 = (((uint32_t)packet.at(ciStart + 8)) << 24u) | (((uint32_t)packet.at(ciStart + 7)) << 16u) | (((uint32_t)packet.at(ciStart + 6)) << 8u) | ((uint32_t)packet.at(ciStart + 5));
      _ellInfo.version2 = packet.at(ciStart + 9);
      _ellInfo.medium2 = packet.at(ciStart + 10);
      _ellInfo.sessionNumberField.raw = ((uint32_t)packet.at(ciStart + 14) << 24u) | ((uint32_t)packet.at(ciStart + 13) << 16u) | ((uint32_t)packet.at(ciStart + 12) << 8u) | packet.at(ciStart + 11);
      _ellInfo.ellEncryption = _ellInfo.sessionNumberField.bitField.encryptionField == 1;
      if (_ellInfo.ellEncryption) {
        _encryptionMode = 1;
        _iv.emplace_back(_ellInfo.communicationControlField.raw);
        _iv.insert(_iv.end(), packet.begin() + ciStart + 11, packet.begin() + ciStart + 15);
        _iv.emplace_back(0); //Frame number - Only needed for bidirectional communication
        _iv.emplace_back(0); //Frame number - Only needed for bidirectional communication - increments with each packet within a session
        _iv.emplace_back(0); //Block counter - Needs to be incremented for each block
        _payload.clear();
        _payload.insert(_payload.end(), packet.begin() + ciStart + 15, packet.end() - 2);
        break;
      }
      ciStart += 17;
      continue;
    } else if (controlInformation == 0x90) //AFL header
    {
      uint8_t aflPos = ciStart + 2;
      if (ciStart + 1 >= packet.size()) break;
      uint8_t aflHeaderSize = packet.at(ciStart + 1);
      ciStart += 2 + aflHeaderSize;
      if (ciStart >= packet.size()) break;

      _aflHeader = AflHeader();
      _aflHeader.fragmentId = packet.at(aflPos++);
      uint8_t fragmentControlField = packet.at(aflPos++);
      _aflHeader.moreFragments = fragmentControlField & 0x40u;
      if (_aflHeader.moreFragments) {
        Gd::out.printWarning("Warning AFL with multiple fragments is unsupported.");
        break;
      }
      if (fragmentControlField & 0x20u) //Has message control field
      {
        _aflHeader.hasMessageControlField = true;
        _aflHeader.messageControlField = packet.at(aflPos++);
        _aflHeader.authenticationType = _aflHeader.messageControlField & 0x0Fu;
        if (_aflHeader.authenticationType != 5) {
          Gd::out.printWarning("Only authentication type 5 is supported at the moment.");
          break;
        }
      }
      if (fragmentControlField & 0x02u) //Has key information field
      {
        _aflHeader.hasKeyInformation = true;
        _aflHeader.keyInformationField = (((uint16_t)packet.at(aflPos + 1)) << 8u) | ((uint16_t)packet.at(aflPos));
        aflPos += 2;
      }
      if (fragmentControlField & 0x08u) //Has message counter
      {
        _aflHeader.hasMessageCounter = true;
        _aflHeader.messageCounter = (((uint32_t)packet.at(aflPos + 3)) << 24) | (((uint32_t)packet.at(aflPos + 2)) << 16) | (((uint32_t)packet.at(aflPos + 1)) << 8) | ((uint32_t)packet.at(aflPos));
        aflPos += 4;
      }
      if (fragmentControlField & 0x04u) //Has MAC
      {
        if (_aflHeader.authenticationType != 5) {
          Gd::out.printWarning("Only authentication type 5 is supported at the moment.");
          break;
        }
        _aflHeader.mac.insert(_aflHeader.mac.end(), packet.begin() + aflPos, packet.begin() + aflPos + 8);
        aflPos += 8;
      }
      if (fragmentControlField & 0x10u) //Has length
      {
        _aflHeader.hasMessageLength = true;
        _aflHeader.messageLength = (((uint16_t)packet.at(aflPos + 1)) << 8) | ((uint16_t)packet.at(aflPos));
        aflPos += 2;
      }
    } else {
      _controlInformation = controlInformation;
      if (hasLongTplHeader()) //Address, manufacturer and medium from header take precedence over outer frame
      {
        _tpduStart = ciStart;
        _secondaryAddress = (((uint32_t)packet.at(ciStart + 4)) << 24) | (((uint32_t)packet.at(ciStart + 3)) << 16) | (((uint32_t)packet.at(ciStart + 2)) << 8) | ((uint32_t)packet.at(ciStart + 1));
        _manufacturerCode = (((uint32_t)packet.at(ciStart + 6)) << 8) | packet.at(ciStart + 5);
        _manufacturer.clear();
        _manufacturer.reserve(3);
        _manufacturer.push_back((char)(((_manufacturerCode >> 10) & 0x1F) + 64));
        _manufacturer.push_back((char)(((_manufacturerCode >> 5) & 0x1F) + 64));
        _manufacturer.push_back((char)((_manufacturerCode & 0x1F) + 64));
        _version = packet.at(ciStart + 7);
        _medium = packet.at(ciStart + 8);

        _messageCounter = packet.at(ciStart + 9);
        _status = packet.at(ciStart + 10);
        _configuration = (((uint16_t)packet.at(ciStart + 11)) << 8) | packet.at(ciStart + 12);
        _encryptionMode = _configuration & 0x1Fu;

        size_t tplPos = 13;
        if (_encryptionMode == 5) {
          _mode5Info = Mode5Info();
          _mode5Info.blockCount = _configuration >> 12u;
        } else if (_encryptionMode == 7) {
          _mode7Info = Mode7Info();
          _mode7Info.messageCounterInTpl = _configuration & 0x20u;
          _mode7Info.blockCount = _configuration >> 12u;

          uint8_t configurationExtension = packet.at(tplPos++);

          _mode7Info.kdf = ((configurationExtension >> 4u) & 3u);
          _mode7Info.keyId = configurationExtension & 0x0Fu;

          if (configurationExtension & 0x40u) //Has version field
          {
            _mode7Info.version = packet.at(tplPos++);
          }

          if (_mode7Info.messageCounterInTpl) {
            _mode7Info.tplMessageCounter = (((uint32_t)packet.at(ciStart + tplPos + 3)) << 24) | (((uint32_t)packet.at(ciStart + tplPos + 2)) << 16) | (((uint32_t)packet.at(ciStart + tplPos + 1)) << 8) | ((uint32_t)packet.at(ciStart + tplPos));
            tplPos += 4;
          }
        }

        _payload.clear();
        _payload.insert(_payload.end(), _packet.begin() + ciStart + tplPos, _packet.end() - 2);
        break; //No more CIs after payload
      } else if (hasShortTplHeader()) {
        _tpduStart = ciStart;
        _manufacturerCode = (((uint32_t)packet.at(5)) << 8) | packet.at(4);
        _manufacturer.clear();
        _manufacturer.reserve(3);
        _manufacturer.push_back((char)(((_manufacturerCode >> 10) & 0x1F) + 64));
        _manufacturer.push_back((char)(((_manufacturerCode >> 5) & 0x1F) + 64));
        _manufacturer.push_back((char)((_manufacturerCode & 0x1F) + 64));
        _secondaryAddress = (((uint32_t)packet.at(9)) << 24u) | (((uint32_t)packet.at(8)) << 16u) | (((uint32_t)packet.at(7)) << 8u) | ((uint32_t)packet.at(6));
        _version = packet.at(10);
        _medium = packet.at(11);

        _messageCounter = packet.at(ciStart + 1);
        _status = packet.at(ciStart + 2);
        _configuration = (((uint16_t)packet.at(ciStart + 3)) << 8u) | packet.at(ciStart + 4);
        _encryptionMode = _configuration & 0x1Fu;

        size_t tplPos = 5;
        if (_encryptionMode == 5) {
          _mode5Info = Mode5Info();
          _mode5Info.blockCount = _configuration >> 12u;
        } else if (_encryptionMode == 7) {
          _mode7Info = Mode7Info();
          _mode7Info.messageCounterInTpl = _configuration & 0x20u;
          _mode7Info.blockCount = _configuration >> 12u;

          uint8_t configurationExtension = packet.at(tplPos++);

          _mode7Info.kdf = ((configurationExtension >> 4u) & 3u);
          _mode7Info.keyId = configurationExtension & 0x0Fu;

          if (configurationExtension & 0x40u) //Has version field
          {
            _mode7Info.version = packet.at(tplPos++);
          }

          if (_mode7Info.messageCounterInTpl) {
            _mode7Info.tplMessageCounter = (((uint32_t)packet.at(ciStart + tplPos + 3)) << 24) | (((uint32_t)packet.at(ciStart + tplPos + 2)) << 16) | (((uint32_t)packet.at(ciStart + tplPos + 1)) << 8) | ((uint32_t)packet.at(ciStart + tplPos));
            tplPos += 4;
          }
        }

        _payload.clear();
        _payload.insert(_payload.end(), _packet.begin() + ciStart + tplPos, _packet.end() - 2);
        break; //No more CIs after payload
      } else {
        Gd::out.printWarning("Warning: Unknown CI: " + BaseLib::HelperFunctions::getHexString(controlInformation));
        break;
      }
    }
  }

  //0: No encryption
  //1: Reserved - We use it for ELL encryption
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
  if (_encryptionMode != 0 && _encryptionMode != 1 && _encryptionMode != 4 && _encryptionMode != 5 && _encryptionMode != 7) {
    Gd::out.printWarning("Warning: Can't process packet, because encryption mode is not supported.");
    return;
  }

  if (_encryptionMode == 4) {
    _iv.clear();
    _iv.resize(16, 0);
  } else if (_encryptionMode == 5) {
    for (int32_t i = 8; i < 16; i++) {
      _iv.push_back(_messageCounter);
    }
  } else if (_encryptionMode == 7) {
    _iv.clear();
    _iv.resize(16, 0);
  } else if (!_ellInfo.ellEncryption) {
    _iv.clear();
  }

  if (_encryptionMode == 0) {
    strip2F(_payload);
    parsePayload();
  }
  _headerValid = true;
}

MbusPacket::~MbusPacket() {
  _packet.clear();
}

std::string MbusPacket::getInfoString() {
  try {
    std::string info = std::string("Type:          ") + (_wireless ? "wMBus" : "M-Bus") + "\n";
    info += "Command:       0x" + BaseLib::HelperFunctions::getHexString(_command) + "\n";
    info += "Length:        " + std::to_string(_length) + "\n";
    info += "Control:       0x" + BaseLib::HelperFunctions::getHexString(_control) + "\n";
    info += "Manufacturer:  " + _manufacturer + "\n";
    info += "Address:       0x" + BaseLib::HelperFunctions::getHexString(_secondaryAddress, 8) + "\n";
    info += "Version:       " + std::to_string(_version) + "\n";
    info += "Medium:        " + std::to_string(_medium) + " (" + getMediumString(_medium) + ")\n";
    info += "Control info:  0x" + std::to_string(_controlInformation) + " (" + getControlInformationString(_controlInformation) + ")\n";
    info += "Counter:       0x" + BaseLib::HelperFunctions::getHexString(_messageCounter) + "\n";
    info += "Status:        0x" + BaseLib::HelperFunctions::getHexString(_status) + "\n";
    if (_wireless) info += "Battery empty: " + std::to_string(batteryEmpty()) + "\n";
    info += "Config:        0x" + BaseLib::HelperFunctions::getHexString(_configuration, 4) + "\n";
    if (_ellInfo.ellEncryption) {
      info += "Encryption:    AES-128-CTR in link layer";
    } else if (_encryptionMode != 0) {
      info += "Encryption:    " + std::string(_encryptionMode == 4 || _encryptionMode == 5 || _encryptionMode == 7 ? "AES (Mode " + std::to_string(_encryptionMode) + ")" : "unknown") + "\n";
    } else {
      info += "Encryption:    none";
    }
    if (_ellInfo.controlInformation != 0) {
      info += "\n ---\n";
      info += getControlInformationString(_ellInfo.controlInformation) + ":\n";
      info += std::string(" - B-field:           ") + (_ellInfo.communicationControlField.bitField.bidirectionalField ? "true" : "false") + "\n";
      info += std::string(" - D-field:           ") + (_ellInfo.communicationControlField.bitField.delayField ? "true" : "false") + "\n";
      info += std::string(" - S-field:           ") + (_ellInfo.communicationControlField.bitField.synchronizedField ? "true" : "false") + "\n";
      info += std::string(" - H-field:           ") + (_ellInfo.communicationControlField.bitField.hopField ? "true" : "false") + "\n";
      info += std::string(" - P-field:           ") + (_ellInfo.communicationControlField.bitField.priorityField ? "true" : "false") + "\n";
      info += std::string(" - A-field:           ") + (_ellInfo.communicationControlField.bitField.accessibilityField ? "true" : "false") + "\n";
      info += std::string(" - R-field:           ") + (_ellInfo.communicationControlField.bitField.repeatedAccessField ? "true" : "false") + "\n";
      info += std::string(" - Access number:     0x") + BaseLib::HelperFunctions::getHexString(_ellInfo.accessNumber) + "\n";
      if (_ellInfo.controlInformation == 0x8D || _ellInfo.controlInformation == 0x8F) {
        info += std::string(" - Encryption:        0x") + BaseLib::HelperFunctions::getHexString(_ellInfo.sessionNumberField.bitField.encryptionField) + "\n";
        info += std::string(" - Time:              0x") + BaseLib::HelperFunctions::getHexString(_ellInfo.sessionNumberField.bitField.timeField) + "\n";
        info += std::string(" - Session:           0x") + BaseLib::HelperFunctions::getHexString(_ellInfo.sessionNumberField.bitField.sessionField) + "\n";
      }
      if (_ellInfo.controlInformation == 0x8E || _ellInfo.controlInformation == 0x8F) {
        info += std::string(" - Manufacturer 2:    0x") + _ellInfo.manufacturer2 + "\n";
        info += std::string(" - Address 2:         0x") + BaseLib::HelperFunctions::getHexString(_ellInfo.address2, 8) + "\n";
        info += std::string(" - Version 2:         ") + std::to_string(_ellInfo.version2) + "\n";
        info += std::string(" - Medium 2:          0x") + std::to_string(_ellInfo.medium2) + " (" + getMediumString(_ellInfo.medium2) + ")\n";
      }
    }
    for (auto &dataRecord: _dataRecords) {
      info += "\n ---\n";
      info += "   DIF: 0x" + BaseLib::HelperFunctions::getHexString(dataRecord.difs.front() & 0x0F) + " (0x" + BaseLib::HelperFunctions::getHexString(dataRecord.difs) + ")" + "\n";
      info += "    - Function:       " + std::to_string((int32_t)dataRecord.difFunction) + "\n";
      info += "    - Storage number: " + std::to_string(dataRecord.storageNumber) + "\n";
      info += "    - Subunit:        " + std::to_string(dataRecord.subunit) + "\n";
      info += "    - Tariff:         " + std::to_string(dataRecord.tariff) + "\n";
      info += "   VIF: 0x" + BaseLib::HelperFunctions::getHexString(dataRecord.vifs) + "\n";
      info += "    - Data start pos: " + std::to_string(dataRecord.dataStart) + "\n";
      info += "    - Data size:      " + std::to_string(dataRecord.dataSize) + "\n";
      info += "    - Data:           " + std::string(dataRecord.data.empty() ? "" : "0x" + BaseLib::HelperFunctions::getHexString(dataRecord.data)) + "\n";
    }

    return info;
  }
  catch (const std::exception &ex) {
    Gd::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return "";
}

std::vector<uint8_t> MbusPacket::getBinary() {
  try {
    if (!_packet.empty()) return _packet;
  }
  catch (const std::exception &ex) {
    Gd::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return {};
}

std::string MbusPacket::getMediumString(uint8_t medium) {
  switch (medium) {
    case 0x00:return "Other";
    case 0x01:return "Oil";
    case 0x02:return "Electricity";
    case 0x03:return "Gas";
    case 0x04:return "Heat (volume measured at return temperature: outlet)";
    case 0x05:return "Steam";
    case 0x06:return "Hot water";
    case 0x07:return "Water";
    case 0x08:return "Heat cost allocator";
    case 0x09:return "Compressed air";
    case 0x0A:return "Cooling load meter (volume measured at return temperature: outlet)";
    case 0x0B:return "Cooling load meter (volume measured at flow temperature: inlet)";
    case 0x0C:return "Heat (volume measured at flow temperature: inlet";
    case 0x0D:return "Heat / cooling load meter";
    case 0x0E:return "Bus / system";
    case 0x0F:return "Unknown";
    case 0x10:return "Reserved for consumption meter (1)";
    case 0x11:return "Reserved for consumption meter (2)";
    case 0x12:return "Reserved for consumption meter (3)";
    case 0x13:return "Reserved for consumption meter (4)";
    case 0x14:return "Calorific value";
    case 0x15:return "Hot water (≥ 90 °C)";
    case 0x16:return "Cold water";
    case 0x17:return "Dual water";
    case 0x18:return "Pressure";
    case 0x19:return "A/D converter";
    case 0x1A:return "Smoke detector";
    case 0x1B:return "Room sensor (e. g. temperature or humidity)";
    case 0x1C:return "Gas detector";
    case 0x1D:return "Reserved for sensors (1)";
    case 0x1E:return "Reserved for sensors (2)";
    case 0x1F:return "Reserved for sensors (3)";
    case 0x20:return "Breaker (electricity)";
    case 0x21:return "Valve (gas or water)";
    case 0x22:return "Reserved for switching devices (1)";
    case 0x23:return "Reserved for switching devices (2)";
    case 0x24:return "Reserved for switching devices (3)";
    case 0x25:return "Customer unit (display device)";
    case 0x26:return "Reserved for customer units (1)";
    case 0x27:return "Reserved for customer units (2)";
    case 0x28:return "Waste water";
    case 0x29:return "Garbage";
    case 0x2A:return "Reserved for carbon dioxide";
    case 0x2B:return "Reserved for environmental meter (1)";
    case 0x2C:return "Reserved for environmental meter (2)";
    case 0x2D:return "Reserved for environmental meter (3)";
    case 0x2E:return "Reserved for environmental meter (4)";
    case 0x2F:return "Reserved for environmental meter (5)";
    case 0x30:return "Reserved for system devices";
    case 0x31:return "Reserved for communication controller";
    case 0x32:return "Reserved for unidirectional repeater";
    case 0x33:return "Reserved for bidirectional repeater";
    case 0x34:return "Reserved for system devices (1)";
    case 0x35:return "Reserved for system devices (2)";
    case 0x36:return "Radio converter (system side)";
    case 0x37:return "Radio converter (meter side)";
    case 0x38:return "Reserved for system devices (1)";
    case 0x39:return "Reserved for system devices (2)";
    case 0x3A:return "Reserved for system devices (3)";
    case 0x3B:return "Reserved for system devices (4)";
    case 0x3C:return "Reserved for system devices (5)";
    case 0x3D:return "Reserved for system devices (6)";
    case 0x3E:return "Reserved for system devices (7)";
    case 0x3F:return "Reserved for system devices (8)";
    default:return "Unknown";
  }
}

std::string MbusPacket::getControlInformationString(uint8_t controlInformation) {
  if (controlInformation >= 0xA0 && controlInformation <= 0xB7) return "Manufacturer specific Application Layer";
  switch (controlInformation) {
    case 0x5A:return "Command to device with short TPL header";
    case 0x5B:return "Command to device with long TPL header";
    case 0x5C:return "Synchronize action (no TPL header)";
    case 0x5D:return "Reserved";
    case 0x5E:return "Reserved";
    case 0x5F:return "Specific usage";
    case 0x60:return "COSEM Data sent by the Readout device to the meter with long Transport Layer";
    case 0x61:return "COSEM Data sent by the Readout device to the meter with short Transport Layer";
    case 0x62:return "Reserved";
    case 0x63:return "Reserved";
    case 0x64:return "Reserved for OBIS-based Data sent by the Readout device to the meter with long Transport Layer";
    case 0x65:return "Reserved for OBIS-based Data sent by the Readout device to the meter with short Transport Layer";
    case 0x66:return "Response regarding the specified application without TPL header";
    case 0x67:return "Response regarding the specified application with short TPL header";
    case 0x68:return "Response regarding the specified application with long TPL header";
    case 0x69:return "EN 13757-3 Application Layer with Format frame and no Transport Layer";
    case 0x6A:return "EN 13757-3 Application Layer with Format frame and with short Transport Layer";
    case 0x6B:return "EN 13757-3 Application Layer with Format frame and with long Transport Layer";
    case 0x6C:return "Clock synchronisation (absolute)";
    case 0x6D:return "Clock synchronisation (relative)";
    case 0x6E:return "Application error from device with short Transport Layer";
    case 0x6F:return "Application error from device with long Transport Layer";
    case 0x70:return "Application error from device without Transport Layer";
    case 0x71:return "Reserved for Alarm Report";
    case 0x72:return "EN 13757-3 Application Layer with long Transport Layer";
    case 0x73:return "EN 13757-3 Application Layer with compact frame and long Transport Layer";
    case 0x74:return "Alarm from device with short Transport Layer";
    case 0x75:return "Alarm from device with long Transport Layer";
    case 0x76:return "Reserved";
    case 0x77:return "Reserved";
    case 0x78:return "EN 13757-3 Application Layer with full frame and no header";
    case 0x79:return "EN 13757-3 Application Layer with compact frame and no header";
    case 0x7A:return "EN 13757-3 Application Layer with short Transport Layer";
    case 0x7B:return "EN 13757-3 Application Layer with compact frame and short header";
    case 0x7C:return "COSEM Application Layer with long Transport Layer";
    case 0x7D:return "COSEM Application Layer with short Transport Layer";
    case 0x7E:return "Reserved for OBIS-based Application Layer with long Transport Layer";
    case 0x7F:return "Reserved for OBIS-based Application Layer with short Transport Layer";
    case 0x80:return "EN 13757-3 Transport Layer (long) from other device to the meter";
    case 0x81:return "Network Layer data";
    case 0x82:return "For future use";
    case 0x83:return "Network Management application";
    case 0x8A:return "EN 13757-3 Transport Layer (short) from the meter to the other device";
    case 0x8B:return "EN 13757-3 Transport Layer (long) from the meter to the other device";
    case 0x8C:return "Extended Link Layer I (2 Byte)";
    case 0x8D:return "Extended Link Layer II (8 Byte)";
    case 0x8E:return "Extended Link Layer III (10 Byte)";
    case 0x8F:return "Extended Link Layer IV (16 Byte)";
    case 0x90:return "AFL header";
    case 0x91:return "Reserved";
    case 0x92:return "Reserved";
    case 0x93:return "Reserved";
    case 0x94:return "Reserved";
    case 0x95:return "Reserved";
    case 0x96:return "Reserved";
    case 0x97:return "Reserved";
    case 0x98:return "Reserved";
    case 0x99:return "Reserved";
    case 0x9A:return "Reserved";
    case 0x9B:return "Reserved";
    case 0x9C:return "Reserved";
    case 0x9D:return "Reserved";
    default:return "Unknown";
  }
}

bool MbusPacket::isTelegramWithoutMeterData() {
  return _controlInformation == 0x69 ||
      _controlInformation == 0x70 ||
      _controlInformation == 0x79;
}

bool MbusPacket::hasShortTplHeader() {
  return _controlInformation == 0x5A ||
      _controlInformation == 0x61 ||
      _controlInformation == 0x65 ||
      _controlInformation == 0x6A ||
      _controlInformation == 0x6E ||
      _controlInformation == 0x74 ||
      _controlInformation == 0x7A ||
      _controlInformation == 0x7B ||
      _controlInformation == 0x7D ||
      _controlInformation == 0x7F ||
      _controlInformation == 0x8A;
}

bool MbusPacket::hasLongTplHeader() {
  return _controlInformation == 0x5B ||
      _controlInformation == 0x60 ||
      _controlInformation == 0x64 ||
      _controlInformation == 0x6B ||
      _controlInformation == 0x6C ||
      _controlInformation == 0x6D ||
      _controlInformation == 0x6F ||
      _controlInformation == 0x72 ||
      _controlInformation == 0x73 ||
      _controlInformation == 0x75 ||
      _controlInformation == 0x7C ||
      _controlInformation == 0x7E ||
      _controlInformation == 0x80 ||
      _controlInformation == 0x84 ||
      _controlInformation == 0x85 ||
      _controlInformation == 0x8B;
}

bool MbusPacket::isFormatTelegram() {
  return _controlInformation == 0x69 ||
      _controlInformation == 0x6A ||
      _controlInformation == 0x6B ||
      (_controlInformation == 0x78 && _manufacturer == "KAM"); //Used as a format telegram by Kamstrup. This is probably a special case though.
}

bool MbusPacket::isCompactDataTelegram() {
  return _controlInformation == 0x73 ||
      _controlInformation == 0x79 ||
      _controlInformation == 0x7B;
}

bool MbusPacket::isDataTelegram() {
  return _controlInformation == 0x72 ||
      _controlInformation == 0x73 ||
      _controlInformation == 0x78 ||
      _controlInformation == 0x79 ||
      _controlInformation == 0x7A ||
      _controlInformation == 0x7B;
}

std::vector<uint8_t> MbusPacket::getPosition(uint32_t position, uint32_t size) {
  try {
    return BaseLib::BitReaderWriter::getPosition(_payload, position, size);
  }
  catch (const std::exception &ex) {
    Gd::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return std::vector<uint8_t>();
}

bool MbusPacket::decrypt(const std::vector<uint8_t> &key) {
  try {
    if (_isDecrypted) return true;
    if (_encryptionMode == 1) { //ELL encryption
      BaseLib::Security::Gcrypt gcrypt(GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_ECB, GCRY_CIPHER_SECURE);
      gcrypt.setKey(key);
      _iv.back() = 0; //Reset block counter
      std::vector<uint8_t> encrypted;
      auto blockSize = _payload.size() / 16 + (_payload.size() % 16 ? 1 : 0);
      for (uint32_t block = 0; block < blockSize; block++) {
        encrypted.clear();
        encrypted.insert(encrypted.end(), _iv.begin(), _iv.end());
        std::vector<uint8_t> encrypted2;
        gcrypt.encrypt(encrypted2, encrypted);
        for (uint8_t j = 0; j < 16; j++) {
          auto payloadIndex = (block * 16) + j;
          if (payloadIndex >= _payload.size()) break;
          _payload.at(payloadIndex) = _payload.at(payloadIndex) ^ encrypted2.at(j);
        }
        _iv.back()++; //Increment block counter
      }

      strip2F(_payload); //Shouldn't contain 2F as no padding is required

      _controlInformation = _payload.at(2);

      std::vector<uint8_t> packet;
      packet.reserve(_packet.size());
      packet.insert(packet.end(), _packet.begin(), _packet.end() - _payload.size());
      packet.insert(packet.end(), _payload.begin(), _payload.end());
      _packet = std::move(packet);

      parsePayload();
      if (!_dataValid) return false;
      _isDecrypted = true;
    } else if (_encryptionMode == 4 || _encryptionMode == 5) {
      if (_mode5Info.blockCount == 0) _mode5Info.blockCount = _payload.size() / 16;

      BaseLib::Security::Gcrypt gcrypt(GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_CBC, GCRY_CIPHER_SECURE);
      gcrypt.setKey(key);
      gcrypt.setIv(_iv);
      std::vector<uint8_t> encrypted;
      encrypted.insert(encrypted.end(), _payload.begin(), _payload.begin() + (_mode5Info.blockCount * 16));
      std::vector<uint8_t> decrypted;
      gcrypt.decrypt(decrypted, encrypted);
      std::vector<uint8_t> unencryptedData;
      if (encrypted.size() < _payload.size()) unencryptedData.insert(unencryptedData.end(), _payload.begin() + encrypted.size(), _payload.end());
      strip2F(decrypted);
      strip2F(unencryptedData);
      _payload.clear();
      _payload.reserve(decrypted.size() + unencryptedData.size());
      _payload.insert(_payload.end(), decrypted.begin(), decrypted.end());
      _payload.insert(_payload.end(), unencryptedData.begin(), unencryptedData.end());
      std::vector<uint8_t> packet;
      packet.reserve(_packet.size());
      packet.insert(packet.end(), _packet.begin(), _packet.end() - _payload.size());
      packet.insert(packet.end(), _payload.begin(), _payload.end());
      _packet = std::move(packet);
      parsePayload();
      _isDecrypted = true;
    } else if (_encryptionMode == 7) {
      //{{{ Check MAC
      if (_aflHeader.mac.empty()) {
        Gd::out.printWarning("Warning: No MAC in packet.");
        return false;
      }

      std::vector<uint8_t> kdfInput;
      kdfInput.reserve(16);
      kdfInput.push_back(1); //MAC from device (Kmac)
      if (_mode7Info.messageCounterInTpl) {
        kdfInput.push_back(_mode7Info.tplMessageCounter & 0xFF);
        kdfInput.push_back((_mode7Info.tplMessageCounter >> 8) & 0xFF);
        kdfInput.push_back((_mode7Info.tplMessageCounter >> 16) & 0xFF);
        kdfInput.push_back(_mode7Info.tplMessageCounter >> 24);
      } else {
        kdfInput.push_back(_aflHeader.messageCounter & 0xFF);
        kdfInput.push_back((_aflHeader.messageCounter >> 8) & 0xFF);
        kdfInput.push_back((_aflHeader.messageCounter >> 16) & 0xFF);
        kdfInput.push_back(_aflHeader.messageCounter >> 24);
      }
      kdfInput.push_back(_secondaryAddress & 0xFF);
      kdfInput.push_back((_secondaryAddress >> 8) & 0xFF);
      kdfInput.push_back((_secondaryAddress >> 16) & 0xFF);
      kdfInput.push_back(_secondaryAddress >> 24);
      kdfInput.resize(16, 7);

      std::vector<uint8_t> iv;
      std::vector<uint8_t> derivedKey;
      try {
        if (!BaseLib::Security::Mac::cmac(key, iv, kdfInput, derivedKey)) {
          Gd::out.printWarning("Warning: Could not generate key.");
          return false;
        }
      }
      catch (BaseLib::Security::GcryptException &ex) {
        Gd::out.printWarning("Warning: Could not generate key: " + std::string(ex.what()));
        return false;
      }

      std::vector<uint8_t> cmacInput;
      cmacInput.reserve(1 + 2 + 4 + 2 + ((_packet.size() - _tpduStart) + 1));
      if (_aflHeader.hasMessageControlField) cmacInput.push_back(_aflHeader.messageControlField);
      if (_aflHeader.hasKeyInformation) {
        cmacInput.push_back(_aflHeader.keyInformationField & 0xFF);
        cmacInput.push_back(_aflHeader.keyInformationField >> 8);
      }
      if (_aflHeader.hasMessageCounter) {
        cmacInput.push_back(_aflHeader.messageCounter & 0xFF);
        cmacInput.push_back((_aflHeader.messageCounter >> 8) & 0xFF);
        cmacInput.push_back((_aflHeader.messageCounter >> 16) & 0xFF);
        cmacInput.push_back(_aflHeader.messageCounter >> 24);
      }
      if (_aflHeader.hasMessageLength) {
        cmacInput.push_back(_aflHeader.messageLength & 0xFF);
        cmacInput.push_back(_aflHeader.messageLength >> 8);
      }
      cmacInput.insert(cmacInput.end(), _packet.begin() + _tpduStart, _packet.end() - 2);

      try {
        std::vector<uint8_t> cmac;
        if (!BaseLib::Security::Mac::cmac(derivedKey, iv, cmacInput, cmac)) {
          Gd::out.printWarning("Warning: Could not generate key.");
          return false;
        }

        cmac.resize(8);
        if (cmac != _aflHeader.mac) {
          Gd::out.printWarning("Warning: CMAC verification failed.");
          return false;
        }
      }
      catch (BaseLib::Security::GcryptException &ex) {
        Gd::out.printWarning("Warning: Could not generate key: " + std::string(ex.what()));
        return false;
      }
      //}}}

      //{{{ Decrypt
      kdfInput.at(0) = 0;
      derivedKey.clear();
      try {
        if (!BaseLib::Security::Mac::cmac(key, iv, kdfInput, derivedKey)) {
          Gd::out.printWarning("Warning: Could not generate key.");
          return false;
        }
      }
      catch (BaseLib::Security::GcryptException &ex) {
        Gd::out.printWarning("Warning: Could not generate key: " + std::string(ex.what()));
        return false;
      }

      BaseLib::Security::Gcrypt gcrypt(GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_CBC, GCRY_CIPHER_SECURE);
      gcrypt.setKey(derivedKey);
      gcrypt.setIv(_iv);
      std::vector<uint8_t> encrypted;
      encrypted.insert(encrypted.end(), _payload.begin(), _payload.begin() + (_mode7Info.blockCount * 16));
      std::vector<uint8_t> decrypted;
      gcrypt.decrypt(decrypted, encrypted);
      if (decrypted.at(0) != 0x2F || decrypted.at(1) != 0x2F) {
        return false; //Two "2F" at the beginning are required to verify correct decryption
      }
      std::vector<uint8_t> unencryptedData;
      if (encrypted.size() < _payload.size()) unencryptedData.insert(unencryptedData.end(), _payload.begin() + encrypted.size(), _payload.end());
      strip2F(decrypted);
      strip2F(unencryptedData);
      _payload.clear();
      _payload.reserve(decrypted.size() + unencryptedData.size());
      _payload.insert(_payload.end(), decrypted.begin(), decrypted.end());
      _payload.insert(_payload.end(), unencryptedData.begin(), unencryptedData.end());
      std::vector<uint8_t> packet;
      packet.reserve(_packet.size());
      packet.insert(packet.end(), _packet.begin(), _packet.end() - _payload.size());
      packet.insert(packet.end(), _payload.begin(), _payload.end());
      _packet = std::move(packet);
      //}}}

      parsePayload();
      if (!_dataValid) return false;
      _isDecrypted = true;
    } else if (_encryptionMode != 0) {
      Gd::out.printWarning("Warning: Encryption mode " + std::to_string(_encryptionMode) + " is currently not supported.");
      return false;
    }
    _isDecrypted = true;
    return true;
  }
  catch (const std::exception &ex) {
    Gd::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

void MbusPacket::strip2F(std::vector<uint8_t> &data) {
  try {
    if (data.empty()) return;
    uint32_t startPos = 0;
    uint32_t endPos = data.size() - 1;
    for (auto &byte: data) {
      if (byte != 0x2F) break;
      startPos++;
    }

    for (uint32_t i = data.size() - 1; i >= 0; i--) {
      if (data[i] != 0x2F) break;
      endPos--;
    }

    if (startPos >= endPos) return;

    std::vector<uint8_t> strippedPayload(data.begin() + startPos, data.begin() + endPos + 1);
    data = std::move(strippedPayload);
  }
  catch (const std::exception &ex) {
    Gd::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void MbusPacket::parsePayload() {
  try {
    _dataRecords.clear();
    if (isCompactDataTelegram()) {
      _formatCrc = (((uint16_t)_payload.at(1)) << 8) | _payload.at(0);
      _dataValid = true;
      return; //Not parseable
    } else if (isFormatTelegram() && _controlInformation != 0x78) { //CI == 0x78 => Special case for Kamstrup
      _formatCrc = (((uint16_t)_payload.at(2)) << 8u) | _payload.at(1);
    }

    //Skip first three bytes for format packets. The format packet starts with length + 2 unknown bytes.
    //Each compact data packet starts with these 2 unknown bytes + 2 additional random unknown bytes
    uint32_t dataPos = _controlInformation == 0x78 ? 7 : 4; //CI == 0x78 => Special case for Kamstrup
    uint32_t nopCount = 0;
    uint32_t pos = 0;
    if (isFormatTelegram()) pos = 3;
    for (; pos < _payload.size();) {
      Gd::out.printDebug("Debug: Parsing payload position " + std::to_string(pos) + "...");
      while (_payload.at(pos) == 0x2F) {
        nopCount++;
        pos++; //Ignore padding byte. Can be within the packet in case unencrypted data follows encrypted data
      }

      DataRecord dataRecord;

      //{{{ Get DIF
      dataRecord.difs.reserve(11);
      dataRecord.difs.push_back(_payload.at(pos++));
      dataRecord.difFunction = (DifFunction)((dataRecord.difs.back() & 0x30u) >> 4u);
      uint32_t count = 0;
      while (dataRecord.difs.back() & 0x80 && pos < _payload.size() && count <= 11) {
        dataRecord.difs.push_back(_payload.at(pos++));
        count++;
      }

      if (pos >= _payload.size()) break;

      if (count > 11) {
        Gd::out.printError("Error: Could not parse packet. Packet contains more than 10 DIFEs");
        break;
      }

      //Get storage number, tariff and subunit
      dataRecord.storageNumber = (dataRecord.difs.front() & 0x40) >> 6;
      if (dataRecord.difs.size() > 1) {
        dataRecord.subunit = 0;
        dataRecord.tariff = 0;
      }
      for (uint32_t i = 1; i < dataRecord.difs.size(); i++) {
        dataRecord.storageNumber |= ((dataRecord.difs.at(i) & 0xF) << (((i - 1) * 4) + 1));
        dataRecord.subunit |= (((dataRecord.difs.at(i) & 0x40) >> 6) << (i - 1));
        dataRecord.tariff |= (((dataRecord.difs.at(i) & 0x30) >> 4) << ((i - 1) * 2));
      }
      //}}}

      if (dataRecord.difs.front() != 0xF) {
        //{{{ Get VIFs
        dataRecord.vifs.reserve(11);
        dataRecord.vifs.push_back(_payload.at(pos++));

        //{{{ Fixes
        if (dataRecord.vifs.front() == 0x6E && _controlInformation == 0x6B && _manufacturer == "EFE" && _medium == 7) {
          dataRecord.vifs.at(0) = 0x6D;
          _payload.at(pos - 1) = 0x6D;
        }
        //}}}

        if ((dataRecord.vifs.front() & 0x7F) == 0x7C) { //Custom string VIF
          if (pos >= _payload.size() || pos + _payload.at(pos) >= _payload.size()) {
            Gd::out.printError("Error: Could not parse packet. Invalid end of data.");
            break;
          }
          uint8_t stringLength = _payload.at(pos);
          dataRecord.vifCustomName.reserve(stringLength);
          for (uint32_t i = pos + stringLength; i > pos; i--) {
            dataRecord.vifCustomName.push_back((char)_payload.at(i));
          }
          pos += stringLength + 1;
        }
        count = 0;
        while ((dataRecord.vifs.back() & 0x80) && pos < _payload.size() && count <= 11) {
          dataRecord.vifs.push_back(_payload.at(pos++));
          count++;
        }

        if (count > 11) {
          Gd::out.printError("Error: Could not parse packet. Packet contains more than 10 VIFEs");
          break;
        }
        //}}}
      }

      dataRecord.dataStart = isFormatTelegram() ? dataPos : pos;
      if (dataRecord.difs.front() == 0x0F || dataRecord.difs.front() == 0x1F) {
        //Manufacturer specific
        dataRecord.dataSize = _payload.size() - pos;
      } else {
        dataRecord.dataSize = getDataSize(dataRecord.difs.front(), pos < _payload.size() ? _payload.at(pos) : 0);
      }
      if (isFormatTelegram()) {
        dataPos += dataRecord.dataSize;
      }
      if (isDataTelegram()) {
        if (pos + dataRecord.dataSize > _payload.size()) break;
        int32_t dataStartPos = pos;
        if ((dataRecord.difs.front() & 0xF) == 0xD) dataStartPos++;
        dataRecord.data.insert(dataRecord.data.end(), _payload.begin() + dataStartPos, _payload.begin() + dataStartPos + dataRecord.dataSize);
        pos += dataRecord.dataSize;
      }

      _dataRecords.push_back(std::move(dataRecord));
    }

    if (_dataRecords.size() < 3) return;

    if (_encryptionMode == 1) { //ELL encryption
      uint16_t crc16 = _crc16.calculate(_payload, 2);
      if ((crc16 >> 8u) != _payload.at(1) || (crc16 & 0xFFu) != _payload.at(0)) {
        Gd::out.printError("Error: Format frame CRC is invalid: " + BaseLib::HelperFunctions::getHexString(getBinary()));
        return;
      }
    }

    if (isFormatTelegram() && _controlInformation != 0x78) { //CI == 0x78 => Special case for Kamstrup
      if (_payload.size() - nopCount - 1 != _payload.at(0)) {
        Gd::out.printError("Error: Payload has wrong length (expected: " + std::to_string(_payload.at(0)) + ", got " + std::to_string(_payload.size() - nopCount - 1) + "): " + BaseLib::HelperFunctions::getHexString(_payload));
        return; //Wrong length byte
      }
      uint16_t crc16 = _crc16.calculate(_payload, 3);
      if ((crc16 >> 8u) != _payload.at(2) || (crc16 & 0xFFu) != _payload.at(1)) {
        Gd::out.printError("Error: Format frame CRC is invalid: " + BaseLib::HelperFunctions::getHexString(getBinary()));
        return;
      }
    }

    _dataValid = true;
  }
  catch (const std::exception &ex) {
    Gd::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

uint32_t MbusPacket::getDataSize(uint8_t dif, uint8_t firstDataByte) {
  dif = dif & 0xF;
  if (dif == 0xD) return firstDataByte + 1;
  return _difSizeMap.at(dif);
}

}
