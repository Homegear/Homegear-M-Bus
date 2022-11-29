/* Copyright 2013-2019 Homegear GmbH */

#ifndef USB300_H_
#define USB300_H_

#include "../MbusPacket.h"
#include "IMbusInterface.h"
#include <homegear-base/BaseLib.h>

namespace Mbus {

class Amber : public IMbusInterface {
 public:
  explicit Amber(std::shared_ptr<BaseLib::Systems::PhysicalInterfaceSettings> settings);
  ~Amber() override;

  void startListening() override;
  void stopListening() override;
  void setup(int32_t userID, int32_t groupID, bool setPermissions) override;

  bool isOpen() override { return _serial && _serial->isOpen() && !_stopped; }
 protected:
  std::unique_ptr<BaseLib::SerialReaderWriter> _serial;
  std::atomic_bool _initComplete;
  std::thread _initThread;

  std::unordered_set<uint8_t> _securityModeWhitelist;

  void init();
  void reconnect();
  void listen();
  bool setParameter(uint8_t address, uint8_t value);
  void RawSend(const std::vector<uint8_t> &packet) override;
  void processPacket(std::vector<uint8_t> &data);
};

}

#endif
