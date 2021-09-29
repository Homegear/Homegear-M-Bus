/* Copyright 2013-2019 Homegear GmbH */

#ifndef HOMEGEAR_MBUS_HGDC_H
#define HOMEGEAR_MBUS_HGDC_H

#include "../MbusPacket.h"
#include "IMbusInterface.h"
#include <homegear-base/BaseLib.h>

namespace Mbus {

class Hgdc : public IMbusInterface {
 public:
  explicit Hgdc(std::shared_ptr<BaseLib::Systems::PhysicalInterfaceSettings> settings);
  ~Hgdc() override;

  void startListening() override;
  void stopListening() override;
  void init();

  bool isOpen() override { return !_stopped && _initComplete; }
 protected:
  int32_t _packetReceivedEventHandlerId = -1;
  std::atomic_bool _initComplete{false};
  std::thread _initThread;

  std::unordered_set<uint8_t> _securityModeWhitelist;

  bool setParameter(uint8_t address, uint8_t value);
  void rawSend(std::vector<uint8_t> &packet) override;
  void processPacket(int64_t familyId, const std::string &serialNumber, const std::vector<uint8_t> &data);
};

}

#endif //HOMEGEAR_ENOCEAN_HOMEGEARGATEWAY_H
