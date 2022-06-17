/* Copyright 2013-2019 Homegear GmbH */

#ifndef HOMEGEAR_MBUS_TCP_H
#define HOMEGEAR_MBUS_TCP_H

#include "../MbusPacket.h"
#include "IMbusInterface.h"
#include <homegear-base/BaseLib.h>

namespace Mbus {

class Tcp : public IMbusInterface {
 public:
  explicit Tcp(std::shared_ptr<BaseLib::Systems::PhysicalInterfaceSettings> settings);
  ~Tcp() override;

  void startListening() override;
  void stopListening() override;

  bool isOpen() override { return !_stopped && _initComplete; }
  void Poll(const std::vector<uint8_t>& primary_addresses, const std::vector<int32_t>& secondary_addresses) override;
 protected:
  enum class PollingInterval {
    off,
    hourly,
    daily,
    weekly,
    monthly
  };

  std::atomic_bool _initComplete{false};
  std::thread listen_thread_;
  std::shared_ptr<BaseLib::TcpSocket> socket_;
  PollingInterval polling_interval_ = PollingInterval::daily;
  int64_t next_polling_ = 0;

  void RawSend(std::vector<uint8_t> &packet) override;
  void listen();
  void ProcessPacket(const std::vector<uint8_t> &packet);
};

}

#endif
