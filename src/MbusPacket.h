/* Copyright 2013-2019 Homegear GmbH */

#ifndef MYPACKET_H_
#define MYPACKET_H_

#include <homegear-base/BaseLib.h>
#include "Crc16.h"

namespace Mbus {

class MbusPacket : public BaseLib::Systems::Packet {
 public:
  enum class DifFunction {
    instantaneousValue = 0,
    maximumValue = 1,
    minimumValue = 2,
    valueDuringErrorState = 3
  };

  struct DataRecord {
    std::vector<uint8_t> difs;
    DifFunction difFunction = DifFunction::instantaneousValue;
    int32_t tariff = -1;
    int32_t subunit = -1;
    int64_t storageNumber = -1;
    std::vector<uint8_t> vifs;
    std::vector<uint8_t> data;
    std::string vifCustomName;
    int32_t dataStart = -1;
    int32_t dataSize = -1;
  };

  struct AflHeader {
    bool hasMessageControlField = false;
    bool hasKeyInformation = false;
    bool hasMessageCounter = false;
    bool hasMessageLength = false;

    uint8_t messageControlField = 0;
    uint8_t fragmentId = 0;
    bool moreFragments = false;
    uint8_t authenticationType = 0;
    uint16_t keyInformationField = 0;
    uint32_t messageCounter = 0;
    std::vector<uint8_t> mac;
    uint16_t messageLength = 0;
  };

  struct EllInfo {
    struct CommunicationControlFieldBitField {
      bool reserved: 1;
      bool repeatedAccessField: 1; //Used by single hop repeaters
      bool accessibilityField: 1; //Is radio reception enabled to receive response?
      bool priorityField: 1; //Set = high priority
      bool hopField: 1; //Set = relayed by repeater?
      bool synchronizedField: 1;
      bool delayField: 1; //Fast (set) or slow (unset) response delay
      bool bidirectionalField: 1;
    };

    union CommunicationControlField {
      uint8_t raw;
      CommunicationControlFieldBitField bitField;
    };

    struct SessionNumberFieldBitField {
      uint8_t sessionField: 4; //Session counter within the minute
      uint32_t timeField: 25; //Relative minute counter to prevent replay attacks
      uint8_t encryptionField: 3; //ELL encryption?
    };

    union SessionNumberField {
      uint32_t raw;
      SessionNumberFieldBitField bitField;
    };

    uint8_t controlInformation = 0;
    CommunicationControlField communicationControlField{0};
    uint8_t accessNumber = 0;
    SessionNumberField sessionNumberField{0};
    bool ellEncryption = false;
    std::string manufacturer2;
    int32_t address2;
    uint8_t version2;
    uint8_t medium2;
  };

  struct Mode5Info {
    uint8_t blockCount = 0;
  };

  struct Mode7Info {
    bool messageCounterInTpl = false;
    uint32_t tplMessageCounter = 0;
    uint8_t blockCount = 0;
    uint8_t version = 0;
    uint8_t kdf = 0;
    uint8_t keyId = 0;
  };

  MbusPacket();
  explicit MbusPacket(const std::vector<uint8_t> &packet);
  ~MbusPacket() override;

  std::string getInfoString();

  bool wireless() const { return _wireless; }
  bool batteryEmpty() { return _status & 4; } //See EN 13757-7 section 7.5.6
  bool permanentError() { return _status & 8; } //See EN 13757-7 section 7.5.6
  bool temporaryError() { return _status & 0x10; } //See EN 13757-7 section 7.5.6
  bool busyError() { return (_status & 3) == 1; } //See EN 13757-7 section 7.5.6
  bool applicationError() { return (_status & 3) == 2; } //See EN 13757-7 section 7.5.6
  bool alarmError() { return (_status & 3) == 3; } //See EN 13757-7 section 7.5.6

  uint8_t length() { return _length; }
  int32_t primaryAddress() { return _primaryAddress; }
  int32_t secondaryAddress() { return _secondaryAddress; }
  uint64_t getDeviceId() { return ((uint64_t)_manufacturerCode << 48u) | ((uint64_t)_version << 40u) | ((uint64_t)_medium << 32u) | (uint32_t)_secondaryAddress; }
  std::string getDeviceIdString() { return _manufacturer + "-" + BaseLib::HelperFunctions::getHexString(_secondaryAddress, 8) + "-" + std::to_string(_version) + "-" + std::to_string(_medium); }
  int32_t getRssi() { return _rssi; }
  uint8_t getControl() { return _control; }
  std::string getManufacturer() { return _manufacturer; }
  uint32_t getManufacturerCode() { return _manufacturerCode; }
  uint8_t getVersion() { return _version; }
  uint8_t getMedium() { return _medium; }
  uint8_t getControlInformation() { return _controlInformation; }
  uint8_t getMessageCounter() { return _messageCounter; }
  uint8_t getStatus() { return _status; }
  uint16_t getConfiguration() { return _configuration; }
  uint8_t getEncryptionMode() { return _encryptionMode; }
  AflHeader getAflHeader() { return _aflHeader; }
  EllInfo getEllInfo() { return _ellInfo; }
  uint16_t getFormatCrc() { return _formatCrc; }
  std::vector<uint8_t> getPayload() { return _payload; }
  std::list<DataRecord> getDataRecords() { return _dataRecords; }
  int32_t dataRecordCount() { return (int32_t)_dataRecords.size(); }

  bool headerValid() { return _headerValid; }
  bool dataValid() { return _dataValid; }
  bool isEncrypted() { return _ellInfo.ellEncryption || _encryptionMode != 0; }
  bool isEllEncrypted() { return _ellInfo.ellEncryption; }
  bool isAflEncrypted() { return _encryptionMode != 0; }
  bool isTelegramWithoutMeterData();
  bool hasShortTplHeader();
  bool hasLongTplHeader();
  bool isFormatTelegram();
  bool isCompactDataTelegram();
  bool isDataTelegram();

  static std::string getMediumString(uint8_t medium);
  std::string getControlInformationString(uint8_t controlInformation);
  std::vector<uint8_t> getBinary();

  std::vector<uint8_t> getPosition(uint32_t position, uint32_t size);
  bool decrypt(const std::vector<uint8_t> &key);
 protected:
  std::array<uint8_t, 16> _difSizeMap;

  std::vector<uint8_t> _packet;
  bool _wireless = true;
  uint8_t _length = 0;
  int32_t _primaryAddress = 0;
  int32_t _secondaryAddress = 0;
  int32_t _rssi = 0;
  uint8_t _command = 0;
  uint8_t _control = 0;
  uint32_t _manufacturerCode = 0;
  std::string _manufacturer;
  uint8_t _version = 0;
  uint8_t _medium = 0;
  uint8_t _controlInformation = 0;
  uint8_t _messageCounter = 0;
  uint8_t _status = 0;
  uint16_t _configuration = 0;
  uint8_t _encryptionMode = 0;
  EllInfo _ellInfo;
  Mode5Info _mode5Info;
  Mode7Info _mode7Info;
  AflHeader _aflHeader;
  uint16_t _formatCrc = 0;
  uint8_t _tpduStart = 0;
  std::vector<uint8_t> _payload;
  std::list<DataRecord> _dataRecords;
  bool _isDecrypted = false;
  bool _headerValid = false;
  bool _dataValid = false;

  std::vector<uint8_t> _iv;

  Crc16 _crc16;

  void strip2F(std::vector<uint8_t> &data);
  void parsePayload();
  uint32_t getDataSize(uint8_t dif, uint8_t firstDataByte);
};

typedef std::shared_ptr<MbusPacket> PMbusPacket;

}
#endif
