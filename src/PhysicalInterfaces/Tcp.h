/* Copyright 2013-2019 Homegear GmbH */

#ifndef HOMEGEAR_MBUS_TCP_H
#define HOMEGEAR_MBUS_TCP_H

#include "../MbusPacket.h"
#include "IMbusInterface.h"
#include <homegear-base/BaseLib.h>

namespace Mbus {

class Tcp : public IMbusInterface {
 public:
  explicit Tcp(const std::shared_ptr<BaseLib::Systems::PhysicalInterfaceSettings>& settings);
  ~Tcp() override;

  void startListening() override;
  void stopListening() override;

  bool isOpen() override { return !_stopped && socket_ && socket_->Connected(); }
  void Poll(const std::vector<uint8_t>& primary_addresses, const std::vector<int32_t>& secondary_addresses, bool fast_mode, bool force) override;
 protected:
  std::atomic_bool _initComplete{false};
  std::thread listen_thread_;
  std::shared_ptr<C1Net::TcpSocket> socket_;
  uint32_t snd_nke_delay_ = 5;
  uint32_t set_temporary_primary_address_delay_ = 10;
  uint32_t req_ud2_delay_ = 10;

  void GetMbusResponse(uint8_t response_type, const std::vector<uint8_t> &request_packet, std::vector<uint8_t> &response_packet, uint32_t timeout = 15000);
  void RawSend(const std::vector<uint8_t> &packet) override;
  void Listen();
  void ProcessPacket(const std::vector<uint8_t> &packet);
};

}

#endif
