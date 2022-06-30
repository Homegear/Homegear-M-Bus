/* Copyright 2013-2019 Homegear GmbH */

#ifndef MYPEER_H_
#define MYPEER_H_

#include "PhysicalInterfaces/IMbusInterface.h"
#include "MbusPacket.h"
#include "VifConverter.h"
#include <homegear-base/BaseLib.h>

using namespace BaseLib;
using namespace BaseLib::DeviceDescription;

namespace Mbus {
class MbusCentral;

class MbusPeer : public BaseLib::Systems::Peer, public BaseLib::Rpc::IWebserverEventSink {
 public:
  MbusPeer(uint32_t parentID, IPeerEventSink *eventHandler);
  MbusPeer(uint64_t id, int32_t address, std::string serialNumber, uint32_t parentID, IPeerEventSink *eventHandler);
  ~MbusPeer() override;
  void init();
  void dispose() override;

  //{{{ Features
  bool wireless() override { return _wireless; }
  //}}}

  //{{{ In table variables
  std::string getPhysicalInterfaceId() const;
  void setPhysicalInterfaceId(std::string);
  std::vector<uint8_t> getAesKey() { return _aesKey; }
  void setAesKey(std::vector<uint8_t> &value) {
    _aesKey = value;
    saveVariable(21, value);
  }
  int32_t getControlInformation() const { return _controlInformation; }
  void setControlInformation(int32_t value) {
    _controlInformation = value;
    saveVariable(22, value);
  }
  int32_t getDataRecordCount() const { return _dataRecordCount; }
  void setDataRecordCount(int32_t value) {
    _dataRecordCount = value;
    saveVariable(23, value);
  }
  int32_t getFormatCrc() const { return _formatCrc; }
  void setFormatCrc(int32_t value) {
    _formatCrc = value;
    saveVariable(24, (int32_t)value);
  }
  int32_t getEncryptionMode() const { return _encryptionMode; }
  void setEncryptionMode(int32_t value) {
    _encryptionMode = value;
    saveVariable(25, (int32_t)value);
  }
  void setLastTime(int32_t value) {
    _lastTime = value;
    saveVariable(26, (int32_t)value);
  }
  void setWireless(bool value) {
    _wireless = value;
    saveVariable(27, (int32_t)value);
  }
  int32_t getPrimaryAddress() const { return _primaryAddress; }
  void setPrimaryAddress(int32_t value) {
    _primaryAddress = value;
    saveVariable(28, (int32_t)value);
  }
  int32_t GetMedium() const { return medium_; }
  void SetMedium(uint8_t value) {
    medium_ = value;
    saveVariable(29, (int32_t)value);
  }
  //}}}

  bool expectsEncryption() { return !_aesKey.empty(); }

  void worker();
  std::string handleCliCommand(std::string command) override;
  void packetReceived(PMbusPacket &packet);

  bool load(BaseLib::Systems::ICentral *central) override;
  void savePeers() override {}

  int32_t getChannelGroupedWith(int32_t channel) override { return -1; }
  int32_t getNewFirmwareVersion() override { return 0; }
  std::string getFirmwareVersionString(int32_t firmwareVersion) override { return "1.0"; }
  bool firmwareUpdateAvailable() override { return false; }

  std::string printConfig();

  /**
   * {@inheritDoc}
   */
  void homegearStarted() override;

  /**
   * {@inheritDoc}
   */
  void homegearShuttingDown() override;

  //RPC methods
  PVariable putParamset(BaseLib::PRpcClientInfo clientInfo, int32_t channel, ParameterGroup::Type::Enum type, uint64_t remoteID, int32_t remoteChannel, PVariable variables, bool checkAcls, bool onlyPushing = false) override;
  PVariable setInterface(BaseLib::PRpcClientInfo clientInfo, std::string interfaceId) override;
  PVariable setValue(BaseLib::PRpcClientInfo clientInfo, uint32_t channel, std::string valueKey, PVariable value, bool wait) override;
  //End RPC methods
 protected:
  struct FrameValue {
    std::list<uint32_t> channels;
    std::vector<uint8_t> value;
  };

  struct FrameValues {
    std::string frameID;
    std::list<uint32_t> paramsetChannels;
    ParameterGroup::Type::Enum parameterSetType;
    std::map<std::string, FrameValue> values;
  };

  //In table variables:
  std::string _physicalInterfaceId;
  std::vector<uint8_t> _aesKey;
  int32_t _controlInformation = -1;
  int32_t _dataRecordCount = -1;
  uint16_t _formatCrc = 0;
  uint8_t _encryptionMode = 0;
  int64_t _lastTime = 0;
  bool _wireless = true;
  int32_t _primaryAddress = -1;
  uint8_t medium_ = 0;
  //End

  bool _shuttingDown = false;

  uint32_t _lastRssiDevice = 0;

  void loadVariables(BaseLib::Systems::ICentral *central, std::shared_ptr<BaseLib::Database::DataTable> &rows) override;
  void saveVariables() override;

  void setRssiDevice(uint8_t rssi);

  std::shared_ptr<BaseLib::Systems::ICentral> getCentral() override;

  void getValuesFromPacket(const PMbusPacket& packet, std::vector<FrameValues> &frameValue);

  PParameterGroup getParameterSet(int32_t channel, ParameterGroup::Type::Enum type) override;

  //{{{ Hooks
  /**
   * {@inheritDoc}
   */
  bool getAllValuesHook2(PRpcClientInfo clientInfo, PParameter parameter, uint32_t channel, PVariable parameters) override;

  /**
   * {@inheritDoc}
   */
  bool getParamsetHook2(PRpcClientInfo clientInfo, PParameter parameter, uint32_t channel, PVariable parameters) override;

  /**
   * {@inheritDoc}
   */
  bool convertFromPacketHook(BaseLib::Systems::RpcConfigurationParameter &parameter, std::vector<uint8_t> &data, PVariable &result) override;
  //}}}

  //{{{ RPC methods
  PVariable getDeviceInfo(BaseLib::PRpcClientInfo clientInfo, std::map<std::string, bool> fields) override;
  //}}}
};

typedef std::shared_ptr<MbusPeer> PMyPeer;

}

#endif
