/* Copyright 2013-2017 Homegear UG (haftungsbeschränkt) */

#ifndef MYPACKET_H_
#define MYPACKET_H_

#include <homegear-base/BaseLib.h>

namespace MyFamily
{

class MyPacket : public BaseLib::Systems::Packet
{
    public:
        enum class DifFunction
        {
            instantaneousValue = 0,
            maximumValue = 1,
            minimumValue = 2,
            valueDuringErrorState = 3
        };

        struct DataRecord
        {
            std::vector<uint8_t> difs;
            DifFunction difFunction = DifFunction::instantaneousValue;
            int32_t tariff = -1;
            int32_t subunit = -1;
            int64_t storageNumber = -1;
            std::vector<uint8_t> vifs;
            std::vector<uint8_t> data;
            int32_t dataStart = -1;
            int32_t dataSize = -1;
        };

        MyPacket();
        MyPacket(std::vector<uint8_t>& packet);
        virtual ~MyPacket();

        std::string getInfoString();

        bool batteryEmpty() { return _status & 4; }

        int32_t getRssi() { return _rssi; }
        uint8_t getControl() { return _control; }
        std::string getManufacturer() { return _manufacturer; }
        uint8_t getVersion() { return _version; }
        uint8_t getMedium() { return _medium; }
        uint8_t getControlInformation() { return _controlInformation; }
        uint8_t getMessageCounter() { return _messageCounter; }
        uint8_t getStatus() { return _status; }
        uint16_t getConfiguration() { return _configuration; }
        uint8_t getEncryptionMode() { return _encryptionMode; }
        std::vector<uint8_t> getPayload() { return _payload; }
        std::list<DataRecord> getDataRecords() { return _dataRecords; }
        int32_t dataRecordCount() { return (int32_t)_dataRecords.size(); }

        bool isEncrypted() { return _encryptionMode != 0; }
        bool isTelegramWithoutMeterData();
        bool isShortTelegram();
        bool isLongTelegram();
        bool isFormatTelegram();
        bool isCompactDataTelegram();
        bool isDataTelegram();

        std::string getMediumString(uint8_t medium);
        std::string getControlInformationString(uint8_t controlInformation);
        std::vector<uint8_t> getBinary();

        std::vector<uint8_t> getPosition(uint32_t position, uint32_t size);
        bool decrypt(std::string key);
    protected:
        std::array<uint8_t, 13> _difSizeMap;

        std::vector<uint8_t> _packet;
        int32_t _rssi = 0;
        uint8_t _command = 0;
        uint8_t _control = 0;
        std::string _manufacturer;
        uint8_t _version = 0;
        uint8_t _medium = 0;
        uint8_t _controlInformation = 0;
        uint8_t _messageCounter = 0;
        uint8_t _status = 0;
        uint16_t _configuration = 0;
        uint8_t _encryptionMode = 0;
        std::vector<uint8_t> _payload;
        int32_t _dataOffset = 0;
        std::list<DataRecord> _dataRecords;

        std::vector<uint8_t> _iv;

        void strip2F();
        void parsePayload();
        uint32_t getDataSize(uint8_t dif, uint8_t firstDataByte);
};

typedef std::shared_ptr<MyPacket> PMyPacket;

}
#endif
