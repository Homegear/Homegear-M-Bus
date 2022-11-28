/* Copyright 2013-2019 Homegear GmbH */

#include "IMbusInterface.h"
#include "../Gd.h"
#include "../MbusPacket.h"

namespace Mbus {

IMbusInterface::IMbusInterface(std::shared_ptr<BaseLib::Systems::PhysicalInterfaceSettings> settings) : IPhysicalInterface(Gd::bl, Gd::family->getFamily(), settings) {
  _bl = Gd::bl;

  if (settings->listenThreadPriority == -1) {
    settings->listenThreadPriority = 0;
    settings->listenThreadPolicy = SCHED_OTHER;
  }
}

IMbusInterface::~IMbusInterface() {

}

void IMbusInterface::GetSerialResponse(std::vector<uint8_t> &request_packet, std::vector<uint8_t> &response_packet) {
  try {
    if (_stopped || request_packet.size() < 4) return;
    response_packet.clear();

    uint8_t responsePacketType = request_packet.at(1) | 0x80;

    std::lock_guard<std::mutex> getResponseGuard(get_response_mutex_);
    std::shared_ptr<Request> request(new Request());
    std::unique_lock<std::mutex> requestsGuard(requests_mutex_);
    requests_[responsePacketType] = request;
    requestsGuard.unlock();
    std::unique_lock<std::mutex> lock(request->mutex);

    try {
      if (_bl->debugLevel >= 5) Gd::out.printDebug("Debug: Sending packet " + BaseLib::HelperFunctions::getHexString(request_packet));
      RawSend(request_packet);
    }
    catch (const BaseLib::SocketOperationException &ex) {
      _out.printError("Error sending packet: " + std::string(ex.what()));
      return;
    }

    if (!request->condition_variable.wait_for(lock, std::chrono::milliseconds(10000), [&] { return request->mutex_ready; })) {
      _out.printError("Error: No response received to packet: " + BaseLib::HelperFunctions::getHexString(request_packet));
    }
    response_packet = request->response;

    requestsGuard.lock();
    requests_.erase(responsePacketType);
    requestsGuard.unlock();
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void IMbusInterface::addAmberCrc8(std::vector<uint8_t> &packet) {
  try {
    if (packet.size() < 4) return;

    uint8_t crc8 = 0;
    for (uint32_t i = 0; i < packet.size() - 1; i++) {
      crc8 = crc8 ^ (uint8_t)packet.at(i);
    }
    packet.back() = crc8;
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void IMbusInterface::addCrc8(std::vector<uint8_t> &packet) {
  try {
    if (packet.size() < 4) return;

    uint8_t crc8 = 0;
    for (uint32_t i = packet.at(0) == 0x10 ? 1 : 4; i < packet.size() - 2; i++) {
      crc8 = crc8 + (uint8_t)packet.at(i);
    }
    packet.at(packet.size() - 2) = crc8;
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void IMbusInterface::raisePacketReceived(std::shared_ptr<BaseLib::Systems::Packet> packet) {
  try {
    PMbusPacket myPacket(std::dynamic_pointer_cast<MbusPacket>(packet));
    if (!myPacket) return;

    BaseLib::Systems::IPhysicalInterface::raisePacketReceived(packet);
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}
}
