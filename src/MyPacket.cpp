/* Copyright 2013-2017 Homegear UG (haftungsbeschränkt) */

#include "MyPacket.h"

#include "GD.h"

namespace MyFamily
{
MyPacket::MyPacket()
{
    _difSizeMap[0] = 0;
    _difSizeMap[1] = 1;
    _difSizeMap[2] = 2;
    _difSizeMap[3] = 3;
    _difSizeMap[4] = 4;
    _difSizeMap[5] = 5;
    _difSizeMap[6] = 6;
    _difSizeMap[7] = 8;
    _difSizeMap[8] = 0;
    _difSizeMap[9] = 1;
    _difSizeMap[10] = 2;
    _difSizeMap[11] = 3;
    _difSizeMap[12] = 4;
    _difSizeMap[13] = 0;
    _difSizeMap[14] = 6;
    _difSizeMap[15] = 0;
}

MyPacket::MyPacket(std::vector<uint8_t>& packet) : MyPacket()
{
    _packet = packet;
	_timeReceived = BaseLib::HelperFunctions::getTime();
    _rssi = packet.at(packet.size() - 2);
    if(_rssi >= 128) _rssi = ((_rssi - 256) / 2) - 74; //From Amber wireless datasheet
    else _rssi = (_rssi / 2) - 74;

    _command = packet.at(1);
    _length = packet.at(2);
    _control = packet.at(3);
    _iv.clear();
    _iv.reserve(16);
    _iv.insert(_iv.end(), packet.begin() + 4, packet.begin() + 12);
    _version = packet.at(10);

    size_t ciStart = 12;
    uint8_t controlInformation = 0;
    for(int32_t i = 0; i < 10; i++)
    {
        if(ciStart >= packet.size()) break;
        controlInformation = packet.at(ciStart);
        if(controlInformation == 0x8C) //ELL I
        {
            ciStart += 3;
            continue;
        }
        else if(controlInformation == 0x8D) //ELL II
        {
            ciStart += 9;
            continue;
        }
        else if(controlInformation == 0x90) //AFL header
        {
            uint8_t aflPos = ciStart + 2;
            if(ciStart + 1 >= packet.size()) break;
            uint8_t aflHeaderSize = packet.at(ciStart + 1);
            ciStart += 2 + aflHeaderSize;
            if(ciStart >= packet.size()) break;

            _aflHeader = AflHeader();
            _aflHeader.fragmentId = packet.at(aflPos++);
            uint8_t fragmentControlField = packet.at(aflPos++);
            _aflHeader.moreFragments = fragmentControlField & 0x40;
            if(_aflHeader.moreFragments)
            {
                GD::out.printWarning("Warning AFL with multiple fragments is unsupported.");
                break;
            }
            if(fragmentControlField & 0x20) //Has message control field
            {
                _aflHeader.hasMessageControlField = true;
                _aflHeader.messageControlField = packet.at(aflPos++);
                _aflHeader.authenticationType = _aflHeader.messageControlField & 0x0F;
                if(_aflHeader.authenticationType != 5)
                {
                    GD::out.printWarning("Only authentication type 5 is supported at the moment.");
                    break;
                }
            }
            if(fragmentControlField & 0x02) //Has key information field
            {
                _aflHeader.hasKeyInformation = true;
                _aflHeader.keyInformationField = (((uint16_t) packet.at(aflPos + 1)) << 8) | ((uint16_t) packet.at(aflPos));
                aflPos += 2;
            }
            if(fragmentControlField & 0x08) //Has message counter
            {
                _aflHeader.hasMessageCounter = true;
                _aflHeader.messageCounter = (((uint32_t) packet.at(aflPos + 3)) << 24) | (((uint32_t) packet.at(aflPos + 2)) << 16) | (((uint32_t) packet.at(aflPos + 1)) << 8) | ((uint32_t) packet.at(aflPos));
                aflPos += 4;
            }
            if(fragmentControlField & 0x04) //Has MAC
            {
                if(_aflHeader.authenticationType != 5)
                {
                    GD::out.printWarning("Only authentication type 5 is supported at the moment.");
                    break;
                }
                _aflHeader.mac.insert(_aflHeader.mac.end(), packet.begin() + aflPos, packet.begin() + aflPos + 8);
                aflPos += 8;
            }
            if(fragmentControlField & 0x10) //Has length
            {
                _aflHeader.hasMessageLength = true;
                _aflHeader.messageLength = (((uint16_t) packet.at(aflPos + 1)) << 8) | ((uint16_t) packet.at(aflPos));
                aflPos += 2;
            }
        }
        else
        {
            _controlInformation = controlInformation;
            if(hasLongTplHeader()) //Address, manufacturer and medium from header take precedence over outer frame
            {
                _tpduStart = ciStart;
                _senderAddress = (((uint32_t) packet.at(ciStart + 4)) << 24) | (((uint32_t) packet.at(ciStart + 3)) << 16) | (((uint32_t) packet.at(ciStart + 2)) << 8) | ((uint32_t) packet.at(ciStart + 1));
                uint32_t value = (((uint32_t) packet.at(ciStart + 6)) << 8) | packet.at(ciStart + 5);
                _manufacturer.clear();
                _manufacturer.reserve(3);
                _manufacturer.push_back((char) (((value >> 10) & 0x1F) + 64));
                _manufacturer.push_back((char) (((value >> 5) & 0x1F) + 64));
                _manufacturer.push_back((char) ((value & 0x1F) + 64));
                _medium = packet.at(ciStart + 8);

                _messageCounter = packet.at(ciStart + 9);
                _status = packet.at(ciStart + 10);
                _configuration = (((uint16_t) packet.at(ciStart + 11)) << 8) | packet.at(ciStart + 12);
                _encryptionMode = _configuration & 0x1F;

                size_t tplPos = 13;
                if(_encryptionMode == 7)
                {
                    _mode7Info = Mode7Info();
                    _mode7Info.messageCounterInTpl = _configuration & 0x20;
                    _mode7Info.blockCount = _configuration >> 12;

                    uint8_t configurationExtension = packet.at(tplPos++);

                    _mode7Info.kdf = ((configurationExtension >> 4) & 3);
                    _mode7Info.keyId = configurationExtension & 0x0F;

                    if(configurationExtension & 0x40) //Has version field
                    {
                        _mode7Info.version = packet.at(tplPos++);
                    }

                    if(_mode7Info.messageCounterInTpl)
                    {
                        _mode7Info.tplMessageCounter = (((uint32_t) packet.at(ciStart + tplPos + 3)) << 24) | (((uint32_t) packet.at(ciStart + tplPos + 2)) << 16) | (((uint32_t) packet.at(ciStart + tplPos + 1)) << 8) | ((uint32_t) packet.at(ciStart + tplPos));
                        tplPos += 4;
                    }
                }

                _payload.clear();
                _payload.insert(_payload.end(), _packet.begin() + ciStart + tplPos, _packet.end() - 2);
                break; //No more CIs after payload
            }
            else if(hasShortTplHeader())
            {
                _tpduStart = ciStart;
                uint32_t value = (((uint32_t) packet.at(5)) << 8) | packet.at(4);
                _manufacturer.clear();
                _manufacturer.reserve(3);
                _manufacturer.push_back((char) (((value >> 10) & 0x1F) + 64));
                _manufacturer.push_back((char) (((value >> 5) & 0x1F) + 64));
                _manufacturer.push_back((char) ((value & 0x1F) + 64));
                _senderAddress = (((uint32_t) packet.at(9)) << 24) | (((uint32_t) packet.at(8)) << 16) | (((uint32_t) packet.at(7)) << 8) | ((uint32_t) packet.at(6));
                _medium = packet.at(11);

                _messageCounter = packet.at(ciStart + 1);
                _status = packet.at(ciStart + 2);
                _configuration = (((uint16_t) packet.at(ciStart + 3)) << 8) | packet.at(ciStart + 4);
                _encryptionMode = _configuration & 0x1F;

                size_t tplPos = 5;
                if(_encryptionMode == 7)
                {
                    _mode7Info = Mode7Info();
                    _mode7Info.messageCounterInTpl = _configuration & 0x20;
                    _mode7Info.blockCount = _configuration >> 12;

                    uint8_t configurationExtension = packet.at(tplPos++);

                    _mode7Info.kdf = ((configurationExtension >> 4) & 3);
                    _mode7Info.keyId = configurationExtension & 0x0F;

                    if(configurationExtension & 0x40) //Has version field
                    {
                        _mode7Info.version = packet.at(tplPos++);
                    }

                    if(_mode7Info.messageCounterInTpl)
                    {
                        _mode7Info.tplMessageCounter = (((uint32_t) packet.at(ciStart + tplPos + 3)) << 24) | (((uint32_t) packet.at(ciStart + tplPos + 2)) << 16) | (((uint32_t) packet.at(ciStart + tplPos + 1)) << 8) | ((uint32_t) packet.at(ciStart + tplPos));
                        tplPos += 4;
                    }
                }

                _payload.clear();
                _payload.insert(_payload.end(), _packet.begin() + ciStart + tplPos, _packet.end() - 2);
                break; //No more CIs after payload
            }
            else
            {
                GD::out.printWarning("Warning: Unknown CI: " + BaseLib::HelperFunctions::getHexString(controlInformation));
                break;
            }
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
    if(_encryptionMode != 0 && _encryptionMode != 4 && _encryptionMode != 5 && _encryptionMode != 7)
    {
        GD::out.printWarning("Warning: Can't process packet, because encryption mode is not supported.");
        return;
    }

    if(_encryptionMode == 4)
    {
        _iv.clear();
        _iv.resize(16, 0);
    }
    else if(_encryptionMode == 5)
    {
        for(int32_t i = 8; i < 16; i++)
        {
            _iv.push_back(_messageCounter);
        }
    }
    else if(_encryptionMode == 7)
    {
        _iv.clear();
        _iv.resize(16, 0);
    }
    else _iv.clear();

    if(_encryptionMode == 0) parsePayload();
    _headerValid = true;
}

MyPacket::~MyPacket()
{
	_packet.clear();
}

std::string MyPacket::getInfoString()
{
    try
    {
        std::string info = "Command:       0x" + BaseLib::HelperFunctions::getHexString(_command) + "\n";
        info +=            "Length:        " + std::to_string(_length) + "\n";
        info +=            "Control:       0x" + BaseLib::HelperFunctions::getHexString(_control) + "\n";
        info +=            "Manufacturer:  " + _manufacturer + "\n";
        info +=            "Address:       0x" + BaseLib::HelperFunctions::getHexString(_senderAddress, 8) + "\n";
        info +=            "Version:       " + std::to_string(_version) + "\n";
        info +=            "Medium:        0x" + std::to_string(_medium) + " (" + getMediumString(_medium) + ")\n";
        info +=            "Control info:  0x" + std::to_string(_controlInformation) + " (" + getControlInformationString(_controlInformation) + ")\n";
        info +=            "Counter:       0x" + BaseLib::HelperFunctions::getHexString(_messageCounter) + "\n";
        info +=            "Status:        0x" + BaseLib::HelperFunctions::getHexString(_status) + "\n";
        info +=            "Battery empty: " + std::to_string(batteryEmpty()) + "\n";
        info +=            "Config:        0x" + BaseLib::HelperFunctions::getHexString(_configuration, 4) + "\n";
        info +=            "Encryption:    " + std::string(_encryptionMode == 4 || _encryptionMode == 5 || _encryptionMode == 7 ? "AES" : (_encryptionMode != 0 ? "unknown" : "none")) + "\n";
        for(auto& dataRecord : _dataRecords)
        {
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
    catch(const std::exception& ex)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return "";
}

std::vector<uint8_t> MyPacket::getBinary()
{
	try
	{
		if(!_packet.empty()) return _packet;
	}
	catch(const std::exception& ex)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return std::vector<uint8_t>();
}

std::string MyPacket::getMediumString(uint8_t medium)
{
    switch(medium)
    {
        case 0x00:
            return "Other";
        case 0x01:
            return "Oil";
        case 0x02:
            return "Electricity";
        case 0x03:
            return "Gas";
        case 0x04:
            return "Heat (volume measured at return temperature: outlet)";
        case 0x05:
            return "Steam";
        case 0x06:
            return "Hot water";
        case 0x07:
            return "Water";
        case 0x08:
            return "Heat cost allocator";
        case 0x09:
            return "Compressed air";
        case 0x0A:
            return "Cooling load meter (volume measured at return temperature: outlet)";
        case 0x0B:
            return "Cooling load meter (volume measured at flow temperature: inlet)";
        case 0x0C:
            return "Heat (volume measured at flow temperature: inlet";
        case 0x0D:
            return "Heat / cooling load meter";
        case 0x0E:
            return "Bus / system";
        case 0x0F:
            return "Unknown";
        case 0x10:
            return "Reserved for consumption meter";
        case 0x11:
            return "Reserved for consumption meter";
        case 0x12:
            return "Reserved for consumption meter";
        case 0x13:
            return "Reserved for consumption meter";
        case 0x14:
            return "Calorific value";
        case 0x15:
            return "Hot water (≥ 90 °C)";
        case 0x16:
            return "Cold water";
        case 0x17:
            return "Dual water";
        case 0x18:
            return "Pressure";
        case 0x19:
            return "A/D converter";
        case 0x1A:
            return "Smoke detector";
        case 0x1B:
            return "Room sensor (e. g. temperature or humidity)";
        case 0x1C:
            return "Gas detector";
        case 0x1D:
            return "Reserved for sensors";
        case 0x1E:
            return "Reserved for sensors";
        case 0x1F:
            return "Reserved for sensors";
        case 0x20:
            return "Breaker (electricity)";
        case 0x21:
            return "Valve (gas or water)";
        case 0x22:
            return "Reserved for switching devices";
        case 0x23:
            return "Reserved for switching devices";
        case 0x24:
            return "Reserved for switching devices";
        case 0x25:
            return "Customer unit (display device)";
        case 0x26:
            return "Reserved for customer units";
        case 0x27:
            return "Reserved for customer units";
        case 0x28:
            return "Waste water";
        case 0x29:
            return "Garbage";
        case 0x2A:
            return "Reserved for carbon dioxide";
        case 0x2B:
            return "Reserved for environmental meter";
        case 0x2C:
            return "Reserved for environmental meter";
        case 0x2D:
            return "Reserved for environmental meter";
        case 0x2E:
            return "Reserved for environmental meter";
        case 0x2F:
            return "Reserved for environmental meter";
        case 0x30:
            return "Reserved for system devices";
        case 0x31:
            return "Reserved for communication controller";
        case 0x32:
            return "Reserved for unidirectional repeater";
        case 0x33:
            return "Reserved for bidirectional repeater";
        case 0x34:
            return "Reserved for system devices";
        case 0x35:
            return "Reserved for system devices";
        case 0x36:
            return "Radio converter (system side)";
        case 0x37:
            return "Radio converter (meter side)";
        case 0x38:
            return "Reserved for system devices";
        case 0x39:
            return "Reserved for system devices";
        case 0x3A:
            return "Reserved for system devices";
        case 0x3B:
            return "Reserved for system devices";
        case 0x3C:
            return "Reserved for system devices";
        case 0x3D:
            return "Reserved for system devices";
        case 0x3E:
            return "Reserved for system devices";
        case 0x3F:
            return "Reserved for system devices";
        default:
            return "Unknown";
    }
}

std::string MyPacket::getControlInformationString(uint8_t controlInformation)
{
    if(controlInformation >= 0xA0 && controlInformation <= 0xB7) return "Manufacturer specific Application Layer";
    switch(controlInformation)
    {
        case 0x5A:
            return "Command to device with short TPL header";
        case 0x5B:
            return "Command to device with long TPL header";
        case 0x5C:
            return "Synchronize action (no TPL header)";
        case 0x5D:
            return "Reserved";
        case 0x5E:
            return "Reserved";
        case 0x5F:
            return "Specific usage";
        case 0x60:
            return "COSEM Data sent by the Readout device to the meter with long Transport Layer";
        case 0x61:
            return "COSEM Data sent by the Readout device to the meter with short Transport Layer";
        case 0x62:
            return "Reserved";
        case 0x63:
            return "Reserved";
        case 0x64:
            return "Reserved for OBIS-based Data sent by the Readout device to the meter with long Transport Layer";
        case 0x65:
            return "Reserved for OBIS-based Data sent by the Readout device to the meter with short Transport Layer";
        case 0x66:
            return "Response regarding the specified application without TPL header";
        case 0x67:
            return "Response regarding the specified application with short TPL header";
        case 0x68:
            return "Response regarding the specified application with long TPL header";
        case 0x69:
            return "EN 13757-3 Application Layer with Format frame and no Transport Layer";
        case 0x6A:
            return "EN 13757-3 Application Layer with Format frame and with short Transport Layer";
        case 0x6B:
            return "EN 13757-3 Application Layer with Format frame and with long Transport Layer";
        case 0x6C:
            return "Clock synchronisation (absolute)";
        case 0x6D:
            return "Clock synchronisation (relative)";
        case 0x6E:
            return "Application error from device with short Transport Layer";
        case 0x6F:
            return "Application error from device with long Transport Layer";
        case 0x70:
            return "Application error from device without Transport Layer";
        case 0x71:
            return "Reserved for Alarm Report";
        case 0x72:
            return "EN 13757-3 Application Layer with long Transport Layer";
        case 0x73:
            return "EN 13757-3 Application Layer with Compact frame and long Transport Layer";
        case 0x74:
            return "Alarm from device with short Transport Layer";
        case 0x75:
            return "Alarm from device with long Transport Layer";
        case 0x76:
            return "Reserved";
        case 0x77:
            return "Reserved";
        case 0x78:
            return "EN 13757-3 Application Layer without Transport Layer (to be defined)";
        case 0x79:
            return "EN 13757-3 Application Layer with Compact frame and no header";
        case 0x7A:
            return "EN 13757-3 Application Layer with short Transport Layer";
        case 0x7B:
            return "EN 13757-3 Application Layer with Compact frame and short header";
        case 0x7C:
            return "COSEM Application Layer with long Transport Layer";
        case 0x7D:
            return "COSEM Application Layer with short Transport Layer";
        case 0x7E:
            return "Reserved for OBIS-based Application Layer with long Transport Layer";
        case 0x7F:
            return "Reserved for OBIS-based Application Layer with short Transport Layer";
        case 0x80:
            return "EN 13757-3 Transport Layer (long) from other device to the meter";
        case 0x81:
            return "Network Layer data";
        case 0x82:
            return "For future use";
        case 0x83:
            return "Network Management application";
        case 0x8A:
            return "EN 13757-3 Transport Layer (short) from the meter to the other device";
        case 0x8B:
            return "EN 13757-3 Transport Layer (long) from the meter to the other device";
        case 0x8C:
            return "Extended Link Layer I (2 Byte)";
        case 0x8D:
            return "Extended Link Layer II (8 Byte)";
        case 0x8E:
            return "Extended Link Layer III";
        case 0x8F:
            return "Extended Link Layer IV";
        case 0x90:
            return "AFL header";
        case 0x91:
            return "Reserved";
        case 0x92:
            return "Reserved";
        case 0x93:
            return "Reserved";
        case 0x94:
            return "Reserved";
        case 0x95:
            return "Reserved";
        case 0x96:
            return "Reserved";
        case 0x97:
            return "Reserved";
        case 0x98:
            return "Reserved";
        case 0x99:
            return "Reserved";
        case 0x9A:
            return "Reserved";
        case 0x9B:
            return "Reserved";
        case 0x9C:
            return "Reserved";
        case 0x9D:
            return "Reserved";
        default:
            return "Unknown";
    }
}

bool MyPacket::isTelegramWithoutMeterData()
{
    return _controlInformation == 0x69 ||
           _controlInformation == 0x70 ||
           _controlInformation == 0x78 ||
           _controlInformation == 0x79;
}

bool MyPacket::hasShortTplHeader()
{
    return _controlInformation == 0x61 ||
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

bool MyPacket::hasLongTplHeader()
{
    return _controlInformation == 0x60 ||
           _controlInformation == 0x64 ||
           _controlInformation == 0x6B ||
           _controlInformation == 0x6F ||
           _controlInformation == 0x72 ||
           _controlInformation == 0x73 ||
           _controlInformation == 0x75 ||
           _controlInformation == 0x7C ||
           _controlInformation == 0x7E ||
           _controlInformation == 0x80 ||
           _controlInformation == 0x8B;
}

bool MyPacket::isFormatTelegram()
{
    return _controlInformation == 0x69 ||
           _controlInformation == 0x6A ||
           _controlInformation == 0x6B;
}

bool MyPacket::isCompactDataTelegram()
{
    return _controlInformation == 0x73 ||
           _controlInformation == 0x79 ||
           _controlInformation == 0x7B;
}

bool MyPacket::isDataTelegram()
{
    return _controlInformation == 0x72 ||
           _controlInformation == 0x73 ||
           _controlInformation == 0x79 ||
           _controlInformation == 0x7A ||
           _controlInformation == 0x7B;
}

std::vector<uint8_t> MyPacket::getPosition(uint32_t position, uint32_t size)
{
	try
	{
		return BaseLib::BitReaderWriter::getPosition(_payload, position, size);
	}
	catch(const std::exception& ex)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return std::vector<uint8_t>();
}

bool MyPacket::decrypt(std::vector<uint8_t>& key)
{
    try
    {
        if(_isDecrypted) return true;
        if(_encryptionMode == 4 || _encryptionMode == 5)
        {
            BaseLib::Security::Gcrypt gcrypt(GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_CBC, GCRY_CIPHER_SECURE);
            gcrypt.setIv(_iv);
            gcrypt.setKey(key);
            std::vector<uint8_t> decrypted;
            gcrypt.decrypt(decrypted, _payload);
            if(decrypted.at(0) != 0x2F || decrypted.at(1) != 0x2F)
            {
                return false; //Two "2F" at the beginning are required to verify correct decryption
            }
            _payload = decrypted;
            std::vector<uint8_t> packet;
            packet.reserve(_packet.size());
            packet.insert(packet.end(), _packet.begin(), _packet.end() - _payload.size());
            packet.insert(packet.end(), _payload.begin(), _payload.end());
            _packet = std::move(packet);
            parsePayload();
            _isDecrypted = true;
        }
        else if(_encryptionMode == 7)
        {
            //{{{ Check MAC
                if(_aflHeader.mac.empty())
                {
                    GD::out.printWarning("Warning: No MAC in packet.");
                    return false;
                }

                std::vector<uint8_t> kdfInput;
                kdfInput.reserve(16);
                kdfInput.push_back(1); //MAC from device (Kmac)
                if(_mode7Info.messageCounterInTpl)
                {
                    kdfInput.push_back(_mode7Info.tplMessageCounter & 0xFF);
                    kdfInput.push_back((_mode7Info.tplMessageCounter >> 8) & 0xFF);
                    kdfInput.push_back((_mode7Info.tplMessageCounter >> 16) & 0xFF);
                    kdfInput.push_back(_mode7Info.tplMessageCounter >> 24);
                }
                else
                {
                    kdfInput.push_back(_aflHeader.messageCounter & 0xFF);
                    kdfInput.push_back((_aflHeader.messageCounter >> 8) & 0xFF);
                    kdfInput.push_back((_aflHeader.messageCounter >> 16) & 0xFF);
                    kdfInput.push_back(_aflHeader.messageCounter >> 24);
                }
                kdfInput.push_back(_senderAddress & 0xFF);
                kdfInput.push_back((_senderAddress >> 8) & 0xFF);
                kdfInput.push_back((_senderAddress >> 16) & 0xFF);
                kdfInput.push_back(_senderAddress >> 24);
                kdfInput.resize(16, 7);

                std::vector<uint8_t> iv;
                std::vector<uint8_t> derivedKey;
                try
                {
                    if(!BaseLib::Security::Mac::cmac(key, iv, kdfInput, derivedKey))
                    {
                        GD::out.printWarning("Warning: Could not generate key.");
                        return false;
                    }
                }
                catch(BaseLib::Security::GcryptException& ex)
                {
                    GD::out.printWarning("Warning: Could not generate key: " + ex.what());
                    return false;
                }

                std::vector<uint8_t> cmacInput;
                cmacInput.reserve(1 + 2 + 4 + 2 + ((_packet.size() - _tpduStart) + 1));
                if(_aflHeader.hasMessageControlField) cmacInput.push_back(_aflHeader.messageControlField);
                if(_aflHeader.hasKeyInformation)
                {
                    cmacInput.push_back(_aflHeader.keyInformationField & 0xFF);
                    cmacInput.push_back(_aflHeader.keyInformationField >> 8);
                }
                if(_aflHeader.hasMessageCounter)
                {
                    cmacInput.push_back(_aflHeader.messageCounter & 0xFF);
                    cmacInput.push_back((_aflHeader.messageCounter >> 8) & 0xFF);
                    cmacInput.push_back((_aflHeader.messageCounter >> 16) & 0xFF);
                    cmacInput.push_back(_aflHeader.messageCounter >> 24);
                }
                if(_aflHeader.hasMessageLength)
                {
                    cmacInput.push_back(_aflHeader.messageLength & 0xFF);
                    cmacInput.push_back(_aflHeader.messageLength >> 8);
                }
                cmacInput.insert(cmacInput.end(), _packet.begin() + _tpduStart, _packet.end() - 2);

                try
                {
                    std::vector<uint8_t> cmac;
                    if(!BaseLib::Security::Mac::cmac(derivedKey, iv, cmacInput, cmac))
                    {
                        GD::out.printWarning("Warning: Could not generate key.");
                        return false;
                    }

                    cmac.resize(8);
                    if(cmac != _aflHeader.mac)
                    {
                        GD::out.printWarning("Warning: CMAC verification failed.");
                        return false;
                    }
                }
                catch(BaseLib::Security::GcryptException& ex)
                {
                    GD::out.printWarning("Warning: Could not generate key: " + ex.what());
                    return false;
                }
            //}}}

            //{{{ Decrypt
                kdfInput.at(0) = 0;
                derivedKey.clear();
                try
                {
                    if(!BaseLib::Security::Mac::cmac(key, iv, kdfInput, derivedKey))
                    {
                        GD::out.printWarning("Warning: Could not generate key.");
                        return false;
                    }
                }
                catch(BaseLib::Security::GcryptException& ex)
                {
                    GD::out.printWarning("Warning: Could not generate key: " + ex.what());
                    return false;
                }

                BaseLib::Security::Gcrypt gcrypt(GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_CBC, GCRY_CIPHER_SECURE);
                gcrypt.setIv(_iv);
                gcrypt.setKey(derivedKey);
                std::vector<uint8_t> encrypted;
                encrypted.insert(encrypted.end(), _payload.begin(), _payload.begin() + (_mode7Info.blockCount * 16));
                std::vector<uint8_t> decrypted;
                gcrypt.decrypt(decrypted, encrypted);
                if(decrypted.at(0) != 0x2F || decrypted.at(1) != 0x2F)
                {
                    return false; //Two "2F" at the beginning are required to verify correct decryption
                }
                std::vector<uint8_t> unencryptedData;
                if(encrypted.size() < _payload.size()) unencryptedData.insert(unencryptedData.end(), _payload.begin() + encrypted.size(), _payload.end());
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
            if(!_dataValid) return false;
            _isDecrypted = true;
        }
        else if(_encryptionMode != 0)
        {
            GD::out.printWarning("Warning: Encryption mode " + std::to_string(_encryptionMode) + " is currently not supported.");
            return false;
        }
        _isDecrypted = true;
        return true;
    }
    catch(const std::exception& ex)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return false;
}

void MyPacket::strip2F()
{
    try
    {
        if(_payload.empty()) return;
        uint32_t startPos = 0;
        uint32_t endPos = _payload.size() - 1;
        for(auto& byte : _payload)
        {
            if(byte != 0x2F) break;
            startPos++;
        }

        for(uint32_t i = _payload.size() - 1; i >= 0; i--)
        {
            if(_payload[i] != 0x2F) break;
            endPos--;
        }

        if(startPos >= endPos) return;

        std::vector<uint8_t> strippedPayload(_payload.begin() + startPos, _payload.begin() + endPos + 1);
        _payload = std::move(strippedPayload);
    }
    catch(const std::exception& ex)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void MyPacket::parsePayload()
{
    try
    {
        _dataRecords.clear();
        strip2F();
        if(isCompactDataTelegram())
        {
            _formatCrc = (((uint16_t)_payload.at(1)) << 8) | _payload.at(0);
            return; //Not parseable
        }
        else if(isFormatTelegram())
        {
            _formatCrc = (((uint16_t)_payload.at(2)) << 8) | _payload.at(1);
        }

        //Skip first three for format packets. The format packet starts with length + 2 unknown bytes.
        //Each compact data packet starts with these 2 unknown bytes + 2 additional random unknown bytes
        uint32_t dataPos = 4;
        for(uint32_t pos = isFormatTelegram() ? 3 : 0; pos < _payload.size();)
        {
            while(_payload.at(pos) == 0x2F) pos++; //Ignore padding byte. Can be within the packet in case unencrypted data follows encrypted data

            DataRecord dataRecord;

            //{{{ Get DIF
            dataRecord.difs.reserve(11);
            dataRecord.difs.push_back(_payload.at(pos++));
            dataRecord.difFunction = (DifFunction)((dataRecord.difs.back() & 0x30) >> 4);
            uint32_t count = 0;
            while(dataRecord.difs.back() & 0x80 && pos < _payload.size() && count <= 11)
            {
                dataRecord.difs.push_back(_payload.at(pos++));
                count++;
            }

            if(pos >= _payload.size()) break;

            if(count > 11)
            {
                GD::out.printError("Error: Could not parse packet. Packet contains more than 10 DIFEs");
                break;
            }

            //Get storage number, tariff and subunit
            dataRecord.storageNumber = (dataRecord.difs.front() & 0x40) >> 6;
            if(dataRecord.difs.size() > 1)
            {
                dataRecord.subunit = 0;
                dataRecord.tariff = 0;
            }
            for(uint32_t i = 1; i < dataRecord.difs.size(); i++)
            {
                dataRecord.storageNumber |= ((dataRecord.difs.at(i) & 0xF) << (((i - 1) * 4) + 1));
                dataRecord.subunit |= (((dataRecord.difs.at(i) & 0x40) >> 6) << (i - 1));
                dataRecord.tariff |= (((dataRecord.difs.at(i) & 0x30) >> 4) << ((i - 1) * 2));
            }
            //}}}

            //{{{ Get VIFs
            dataRecord.vifs.reserve(11);
            dataRecord.vifs.push_back(_payload.at(pos++));

                //{{{ Fixes
                if(dataRecord.vifs.front() == 0x6E && _controlInformation == 0x6B && _manufacturer == "EFE" && _medium == 7)
                {
                    dataRecord.vifs.at(0) = 0x6D;
                    _payload.at(pos - 1) = 0x6D;
                }
                //}}}

            count = 0;
            while(dataRecord.vifs.back() & 0x80 && pos < _payload.size() && count <= 11)
            {
                dataRecord.vifs.push_back(_payload.at(pos++));
                count++;
            }

            if(count > 11)
            {
                GD::out.printError("Error: Could not parse packet. Packet contains more than 10 VIFEs");
                break;
            }

            if(dataRecord.vifs.front() == 0x0F || dataRecord.vifs.front() == 0x1F)
            {
                GD::out.printInfo("Info: The packet contains manufacturer specific data which is currently not supported.");
                break;
            }
            //}}}

            dataRecord.dataStart = isFormatTelegram() ? dataPos : pos;
            dataRecord.dataSize = getDataSize(dataRecord.difs.front(), pos < _payload.size() ? _payload.at(pos) : 0);
            if(isFormatTelegram()) dataPos += dataRecord.dataSize;
            else
            {
                if(pos + dataRecord.dataSize > _payload.size()) break;
                dataRecord.data.insert(dataRecord.data.end(), _payload.begin() + pos, _payload.begin() + pos + dataRecord.dataSize);
                pos += dataRecord.dataSize;
            }

            _dataRecords.push_back(std::move(dataRecord));
        }

        if(_dataRecords.size() < 3) return;

        if(isFormatTelegram())
        {
            if(_payload.size() - 1 != _payload.at(0)) return; //Wrong length byte
            uint16_t crc16 = _crc16.calculate(_payload, 3);
            if((crc16 >> 8) != _payload.at(2) || (crc16 & 0xFF) != _payload.at(1))
            {
                GD::out.printError("Error: Format frame CRC is invalid: " + BaseLib::HelperFunctions::getHexString(getBinary()));
                return;
            }
        }

        _dataValid = true;
    }
    catch(const std::exception& ex)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        GD::out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

uint32_t MyPacket::getDataSize(uint8_t dif, uint8_t firstDataByte)
{
    dif = dif & 0xF;
    if(dif == 0xD) return firstDataByte + 1;
    return _difSizeMap.at(dif);
}

}
