/* Copyright 2013-2017 Homegear UG (haftungsbeschränkt) */

#ifndef MYPACKET_H_
#define MYPACKET_H_

#include <homegear-base/BaseLib.h>

namespace MyFamily
{

class MyPacket : public BaseLib::Systems::Packet
{
    public:
        MyPacket();
        MyPacket(std::vector<uint8_t>& packet);
        virtual ~MyPacket();

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

        std::string getMediumString(uint8_t medium);
        std::string getControlInformationString(uint8_t controlInformation);
        std::vector<uint8_t> getBinary();

        std::vector<uint8_t> getPosition(uint32_t position, uint32_t size);
    protected:
        std::vector<uint8_t> _packet;
        int32_t _rssi = 0;
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

        std::vector<uint8_t> _iv;

        bool isTelegramWithoutMeterData();
        bool isShortTelegram();
        bool isLongTelegram();
};

typedef std::shared_ptr<MyPacket> PMyPacket;

}
#endif
