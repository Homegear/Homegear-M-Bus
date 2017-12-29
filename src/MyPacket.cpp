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
    _controlInformation = packet.at(12);

    if(isLongTelegram()) //Address, manufacturer and medium from header take precedence over outer frame
    {
        _senderAddress = (((uint32_t)packet.at(16)) << 24) | (((uint32_t)packet.at(15)) << 16) | (((uint32_t)packet.at(14)) << 8) | ((uint32_t)packet.at(13));
        uint32_t value = (((uint32_t)packet.at(18)) << 8) | packet.at(17);
        _manufacturer.clear();
        _manufacturer.reserve(3);
        _manufacturer.push_back((char)(((value >> 10) & 0x1F) + 64));
        _manufacturer.push_back((char)(((value >> 5) & 0x1F) + 64));
        _manufacturer.push_back((char)((value & 0x1F) + 64));
        _medium = packet.at(20);

        _dataOffset = 21;
        _messageCounter = packet.at(21);
        _status = packet.at(22);
        _configuration = (((uint16_t)packet.at(23)) << 8) | packet.at(24);
        _encryptionMode = _configuration & 0x0F;
        _payload.clear();
        _payload.insert(_payload.end(), _packet.begin() + 25, _packet.end() - 2);
    }
    else if(isShortTelegram())
    {
        uint32_t value = (((uint32_t)packet.at(5)) << 8) | packet.at(4);
        _manufacturer.clear();
        _manufacturer.reserve(3);
        _manufacturer.push_back((char)(((value >> 10) & 0x1F) + 64));
        _manufacturer.push_back((char)(((value >> 5) & 0x1F) + 64));
        _manufacturer.push_back((char)((value & 0x1F) + 64));
        _senderAddress = (((uint32_t)packet.at(9)) << 24) | (((uint32_t)packet.at(8)) << 16) | (((uint32_t)packet.at(7)) << 8) | ((uint32_t)packet.at(6));
        _medium = packet.at(11);

        _dataOffset = 13;
        _messageCounter = packet.at(13);
        _status = packet.at(14);
        _configuration = (((uint16_t)packet.at(15)) << 8) | packet.at(16);
        _encryptionMode = _configuration & 0x0F;
        _payload.clear();
        _payload.insert(_payload.end(), _packet.begin() + 17, _packet.end() - 2);
    }

    //0: No encryption
    //1: Reserved
    //2: DES encryption with CBC; IV is zero (deprecated)
    //3: DES encryption with CBC; IV is not zero (deprecated)
    //4: AES encryption with CBC; IV is zero
    //5: AES encryption with CBC; IV is not zero
    //6 - 15: Reserved
    if(_encryptionMode != 0 && _encryptionMode != 4 && _encryptionMode != 5)
    {
        GD::out.printWarning("Warning: Can't process packet, because DES encryption mode is not supported.");
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
    else _iv.clear();

    if(_encryptionMode == 0) parsePayload();
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
        info +=            "Encryption:    " + std::string(_encryptionMode == 4 || _encryptionMode == 5 ? "AES" : (_encryptionMode != 0 ? "unknown" : "none")) + "\n";
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
        case 0x60:
            return "COSEM Data sent by the Readout device to the meter with long Transport Layer";
        case 0x61:
            return "COSEM Data sent by the Readout device to the meter with short Transport Layer";
        case 0x64:
            return "Reserved for OBIS-based Data sent by the Readout device to the meter with long Transport Layer";
        case 0x65:
            return "Reserved for OBIS-based Data sent by the Readout device to the meter with short Transport Layer";
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
            return "Extended Link Layer II (8 Byte";
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

bool MyPacket::isShortTelegram()
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

bool MyPacket::isLongTelegram()
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

std::vector<uint8_t> MyPacket::getPosition(uint32_t position, uint32_t size)
{
	try
	{
		return BaseLib::BitReaderWriter::getPosition(_packet, position + (_dataOffset * 8), size);
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

void MyPacket::decrypt(std::string key)
{
    try
    {
        if(_encryptionMode == 4 || _encryptionMode == 5)
        {
            BaseLib::Security::Gcrypt gcrypt(GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_CBC, GCRY_CIPHER_SECURE);
            gcrypt.setIv(_iv);
            std::vector<uint8_t> binaryKey = GD::bl->hf.getUBinary(key);
            gcrypt.setKey(binaryKey);
            std::vector<uint8_t> decrypted;
            gcrypt.decrypt(decrypted, _payload);
            _payload = decrypted;
            parsePayload();
        }
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

void MyPacket::strip2F()
{
    try
    {
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
        if(isCompactDataTelegram()) return; //Not parseable

        //Skip first three for format packets. The format packet starts with length + 2 unknown bytes.
        //Each compact data packet starts with these 2 unknown bytes + 2 additional random unknown bytes
        uint32_t dataPos = 4;
        for(uint32_t pos = isFormatTelegram() ? 3 : 0; pos < _payload.size();)
        {
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

            //{{{ Get VIF
            dataRecord.vifs.reserve(11);
            dataRecord.vifs.push_back(_payload.at(pos++));
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
