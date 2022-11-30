/* Copyright 2013-2019 Homegear GmbH */

#ifndef MYCENTRAL_H_
#define MYCENTRAL_H_

#include "MbusPeer.h"
#include "MbusPacket.h"
#include "DescriptionCreator.h"
#include <homegear-base/BaseLib.h>

#include <memory>
#include <mutex>
#include <string>

namespace Mbus {

class MbusCentral : public BaseLib::Systems::ICentral {
 public:
  explicit MbusCentral(ICentralEventSink *eventHandler);
  MbusCentral(uint32_t deviceType, std::string serialNumber, ICentralEventSink *eventHandler);
  ~MbusCentral() override;
  void dispose(bool wait = true) override;

  std::string handleCliCommand(std::string command);
  bool onPacketReceived(std::string &senderId, std::shared_ptr<BaseLib::Systems::Packet> packet) override;

  uint64_t getPeerIdFromSerial(std::string &serialNumber) {
    std::shared_ptr<MbusPeer> peer = getPeer(serialNumber);
    if (peer) return peer->getID(); else return 0;
  }
  PMyPeer getPeer(uint64_t id);
  PMyPeer getPeer(int32_t address);
  PMyPeer getPeer(std::string serialNumber);

  PVariable createDevice(BaseLib::PRpcClientInfo clientInfo, int32_t deviceType, std::string secondary_address, int32_t primary_address, int32_t firmwareVersion, std::string interfaceId) override;
  PVariable deleteDevice(BaseLib::PRpcClientInfo clientInfo, std::string serialNumber, int32_t flags) override;
  PVariable deleteDevice(BaseLib::PRpcClientInfo clientInfo, uint64_t peerId, int32_t flags) override;
  PVariable getSniffedDevices(BaseLib::PRpcClientInfo clientInfo) override;
  PVariable invokeFamilyMethod(BaseLib::PRpcClientInfo clientInfo, std::string &method, PArray parameters) override;
  PVariable setInstallMode(BaseLib::PRpcClientInfo clientInfo, bool on, uint32_t duration, BaseLib::PVariable metadata, bool debugOutput = true) override;
  PVariable startSniffing(BaseLib::PRpcClientInfo clientInfo) override;
  PVariable stopSniffing(BaseLib::PRpcClientInfo clientInfo) override;
 protected:
  enum class PollingInterval {
    kOff,
    kQuarterHourly,
    kHourly,
    kDaily,
    kWeekly,
    kMonthly
  };

  std::map<std::string, std::function<BaseLib::PVariable(const BaseLib::PRpcClientInfo &clientInfo, const BaseLib::PArray &parameters)>> _localRpcMethods;

  bool _sniff = false;
  std::mutex _sniffedPacketsMutex;
  std::map<uint64_t, std::vector<PMbusPacket>> _sniffedPackets;

  std::atomic_bool _stopPairingModeThread;
  std::mutex _pairingModeThreadMutex;
  std::thread _pairingModeThread;
  std::mutex _devicesToPairMutex;
  std::unordered_map<int32_t, std::string> _devicesToPair;
  std::mutex _pairMutex;
  DescriptionCreator _descriptionCreator;

  std::atomic_bool _stopWorkerThread;
  std::thread _workerThread;

  //{{{ Polling
  std::atomic<int64_t> last_poll_{0};
  //}}}

  virtual void init();
  virtual void worker();
  void loadPeers() override;
  void savePeers(bool full) override;
  void loadVariables() override;
  void saveVariables() override;
  std::shared_ptr<MbusPeer> createPeer(uint64_t deviceType, int32_t address, std::string serialNumber, bool save = true);
  void deletePeer(uint64_t id);

  void pairingModeTimer(int32_t duration, bool debugOutput = true);
  void pairDevice(const PMbusPacket& packet, std::vector<uint8_t> &key);

  void PollPeers(bool use_secondary_address);

  //{{{ Family RPC methods
  BaseLib::PVariable getPrimaryAddress(const BaseLib::PRpcClientInfo &clientInfo, const BaseLib::PArray &parameters);
  BaseLib::PVariable setPrimaryAddress(const BaseLib::PRpcClientInfo &clientInfo, const BaseLib::PArray &parameters);
  BaseLib::PVariable poll(const BaseLib::PRpcClientInfo &clientInfo, const BaseLib::PArray &parameters);
  BaseLib::PVariable processPacket(const BaseLib::PRpcClientInfo &clientInfo, const BaseLib::PArray &parameters);
  //}}}
};

}

#endif
