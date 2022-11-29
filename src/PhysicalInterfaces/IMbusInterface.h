/* Copyright 2013-2019 Homegear GmbH */

#ifndef IENOCEANINTERFACE_H_
#define IENOCEANINTERFACE_H_

#include <homegear-base/BaseLib.h>

namespace Mbus {

class IMbusInterface : public BaseLib::Systems::IPhysicalInterface {
 public:
  IMbusInterface(std::shared_ptr<BaseLib::Systems::PhysicalInterfaceSettings> settings);
  ~IMbusInterface() override;

  void startListening() override;
  void stopListening() override;

  void sendPacket(std::shared_ptr<BaseLib::Systems::Packet> packet) override {}
  virtual void Poll(const std::vector<uint8_t>& primary_addresses, const std::vector<int32_t>& secondary_addresses) {}
 protected:
  struct Request {
    std::mutex mutex;
    std::condition_variable condition_variable;
    bool mutex_ready = false;
    std::vector<uint8_t> response;
  };

  BaseLib::SharedObjects *_bl = nullptr;
  BaseLib::Output _out;

  std::mutex get_response_mutex_;

  std::mutex requests_mutex_;
  std::map<uint8_t, std::shared_ptr<Request>> requests_;

  void GetSerialResponse(std::vector<uint8_t> &request_packet, std::vector<uint8_t> &response_packet);
  virtual void RawSend(const std::vector<uint8_t> &packet) {}
  void addAmberCrc8(std::vector<uint8_t> &packet);
  void addCrc8(std::vector<uint8_t> &packet);

  void raisePacketReceived(std::shared_ptr<BaseLib::Systems::Packet> packet) override;
};

}

#endif
