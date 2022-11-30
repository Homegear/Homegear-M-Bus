/* Copyright 2013-2019 Homegear GmbH */

#include "Tcp.h"
#include "../Gd.h"

namespace Mbus {

Tcp::Tcp(const std::shared_ptr<BaseLib::Systems::PhysicalInterfaceSettings>& settings) : IMbusInterface(settings) {
  _settings = settings;
  _out.init(Gd::bl);
  _out.setPrefix(_out.getPrefix() + "Tcp \"" + settings->id + "\": ");

  signal(SIGPIPE, SIG_IGN);

  _stopped = true;
}

Tcp::~Tcp() {
  stopListening();
}

void Tcp::startListening() {
  try {
    IMbusInterface::startListening();

    if (_settings->host.empty()) {
      _out.printError("Error: No hostname or ip address specified. Please set it in \"mbus.conf\".");
      return;
    }

    if (_settings->port.empty()) {
      _out.printError("Error: No port specified. Please set it in \"mbus.conf\".");
      return;
    }

    socket_ = std::make_shared<BaseLib::TcpSocket>(Gd::bl, _settings->host, _settings->port, !_settings->caFile.empty(), _settings->caFile, true);
    socket_->setConnectionRetries(1);
    socket_->setReadTimeout(100000);

    _stopped = false;

    if (listen_thread_.joinable()) listen_thread_.join();
    listen_thread_ = std::thread(&Tcp::Listen, this);
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Tcp::stopListening() {
  try {
    _stopped = true;
    _bl->threadManager.join(listen_thread_);
    IMbusInterface::stopListening();
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Tcp::Poll(const std::vector<uint8_t> &primary_addresses, const std::vector<int32_t> &secondary_addresses) {
  try {
    for (auto &address: primary_addresses) {
      std::vector<uint8_t> request_packet{0x10, 0x40, address, 0, 0x16};
      addCrc8(request_packet);

      std::vector<uint8_t> response_packet;
      GetMbusResponse(0xE5, request_packet, response_packet);
      if (response_packet.empty()) continue;

      for (uint32_t i = 0; i < 50; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (_stopped) return;
      }

      request_packet.at(1) = 0x7B;
      addCrc8(request_packet);

      response_packet.clear();
      GetMbusResponse(0x68, request_packet, response_packet);
      if (!response_packet.empty()) {
        PMbusPacket mbus_packet = std::make_shared<MbusPacket>(response_packet);
        if (mbus_packet->headerValid()) {
          raisePacketReceived(mbus_packet);
        } else _out.printWarning("Warning: Could not parse packet: " + BaseLib::HelperFunctions::getHexString(response_packet));
      }

      for (uint32_t i = 0; i < 50; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (_stopped) return;
      }
    }

    for (auto &address: secondary_addresses) {
      std::vector<uint8_t> request_packet_1{0x10, 0x40, 0xFF, 0, 0x16};
      addCrc8(request_packet_1);

      RawSend(request_packet_1);

      for (uint32_t i = 0; i < 50; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (_stopped) return;
      }

      //Packet to temporarily set primary address for this device to 0xFD.
      std::vector<uint8_t> request_packet_2{0x68, 0x0B, 0x0B, 0x68, 0x73, 0xFD, 0x52, (uint8_t)address, (uint8_t)(address >> 8), (uint8_t)(address >> 16), (uint8_t)(address >> 24), 0xFF, 0xFF, 0xFF, 0xFF, 0, 0x16};
      addCrc8(request_packet_2);

      std::vector<uint8_t> response_packet;
      GetMbusResponse(0xE5, request_packet_2, response_packet);
      if (response_packet.empty()) continue;

      for (uint32_t i = 0; i < 50; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (_stopped) return;
      }

      std::vector<uint8_t> request_packet_3{0x10, 0x7B, 0xFD, 0, 0x16};
      addCrc8(request_packet_3);

      response_packet.clear();
      GetMbusResponse(0x68, request_packet_3, response_packet);
      if (!response_packet.empty()) {
        PMbusPacket mbus_packet = std::make_shared<MbusPacket>(response_packet);
        if (mbus_packet->headerValid()) {
          raisePacketReceived(mbus_packet);
        } else _out.printWarning("Warning: Could not parse packet: " + BaseLib::HelperFunctions::getHexString(response_packet));
      }

      for (uint32_t i = 0; i < 50; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (_stopped) return;
      }
    }
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Tcp::GetMbusResponse(uint8_t response_type, const std::vector<uint8_t> &request_packet, std::vector<uint8_t> &response_packet) {
  try {
    if (_stopped || request_packet.empty()) return;
    response_packet.clear();

    std::lock_guard<std::mutex> get_response_guard(get_response_mutex_);
    std::shared_ptr<Request> request(new Request());
    std::unique_lock<std::mutex> requests_guard(requests_mutex_);
    requests_[response_type] = request;
    requests_guard.unlock();
    std::unique_lock<std::mutex> wait_lock(request->mutex);

    try {
      RawSend(request_packet);
    }
    catch (const BaseLib::SocketOperationException &ex) {
      _out.printError("Error sending packet: " + std::string(ex.what()));
      return;
    }

    auto start_time = BaseLib::HelperFunctions::getTime();
    while (!request->condition_variable.wait_for(wait_lock, std::chrono::milliseconds(1000), [&] {
      return request->mutex_ready || _stopped || BaseLib::HelperFunctions::getTime() - start_time > 60000;
    }));

    if (!request->mutex_ready) {
      _out.printError("Error: No response received to packet: " + BaseLib::HelperFunctions::getHexString(request_packet));
    }

    response_packet = request->response;

    requests_guard.lock();
    requests_.erase(response_type);
    requests_guard.unlock();
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Tcp::RawSend(const std::vector<uint8_t> &packet) {
  try {
    if (!socket_) {
      _out.printWarning("Warning: Could not send packet as the socket is not open.");
      return;
    }

    if (Gd::bl->debugLevel >= 4) _out.printInfo("Info: Sending packet " + BaseLib::HelperFunctions::getHexString(packet));
    socket_->proofwrite((char *)packet.data(), packet.size());
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Tcp::Listen() {
  try {
    BaseLib::TcpSocket::TcpPacket data;
    std::vector<uint8_t> buffer(4096);
    uint32_t bytes_received = 0;
    int64_t last_activity = 0;

    while (!_stopped) {
      try {
        if (!socket_->connected()) {
          socket_->open();
          if (!socket_->connected()) {
            _out.printWarning("Warning: Not connected to socket.");
            for (int32_t i = 0; i < 10; i++) {
              std::this_thread::sleep_for(std::chrono::milliseconds(100));
              if (_stopped) return;
            }
            continue;
          }
          _out.printInfo("Info: Connected.");
        }

        bytes_received = socket_->proofread((char *)buffer.data(), (int32_t)buffer.size());

        if (BaseLib::HelperFunctions::getTime() - last_activity > 2000 && !data.empty()) {
          _out.printWarning("Warning: Discarding packet buffer: " + BaseLib::HelperFunctions::getHexString(data));
          data.clear();
        }
        last_activity = BaseLib::HelperFunctions::getTime();

        if (bytes_received > 0) {
          if (Gd::bl->debugLevel >= 4) _out.printInfo("Info: Raw packet received: " + BaseLib::HelperFunctions::getHexString(buffer.data(), bytes_received));
          uint32_t processed_bytes = 0;
          while (processed_bytes < bytes_received) {
            if (data.empty()) {
              if (buffer.at(0 + processed_bytes) == 0xE5) {
                ProcessPacket(std::vector<uint8_t>{0xE5});
                processed_bytes++;
                continue;
              } else if (buffer.at(0 + processed_bytes) == 0x10) {
                if (bytes_received - processed_bytes < 5) {
                  data.insert(data.end(), buffer.begin() + processed_bytes, buffer.begin() + processed_bytes + (bytes_received - processed_bytes));
                  processed_bytes += bytes_received;
                } else {
                  ProcessPacket(std::vector<uint8_t>(buffer.begin() + processed_bytes, buffer.begin() + processed_bytes + 5));
                  processed_bytes += 5;
                }
              } else if (buffer.at(0 + processed_bytes) == 0x68) {
                if (bytes_received - processed_bytes < 2) {
                  data.insert(data.end(), buffer.begin() + processed_bytes, buffer.begin() + processed_bytes + (bytes_received - processed_bytes));
                  processed_bytes += bytes_received;
                } else {
                  uint32_t packet_size = buffer.at(1 + processed_bytes) + 6;
                  if (bytes_received - processed_bytes < packet_size) {
                    data.insert(data.end(), buffer.begin() + processed_bytes, buffer.begin() + processed_bytes + (bytes_received - processed_bytes));
                    processed_bytes += bytes_received;
                  } else {
                    ProcessPacket(std::vector<uint8_t>(buffer.begin() + processed_bytes, buffer.begin() + processed_bytes + packet_size));
                    processed_bytes += packet_size;
                  }
                }
              } else {
                processed_bytes += bytes_received;
                data.clear();
              }
            } else {
              if (data.at(0) == 0x10) {
                if (data.size() + (bytes_received - processed_bytes) < 5) {
                  data.insert(data.end(), buffer.begin() + processed_bytes, buffer.begin() + processed_bytes + (bytes_received - processed_bytes));
                  processed_bytes += bytes_received;
                } else {
                  data.insert(data.end(), buffer.begin() + processed_bytes, buffer.begin() + processed_bytes + (5 - (int32_t)data.size()));
                  processed_bytes += 5 - data.size();
                  ProcessPacket(data);
                  data.clear();
                }
              } else {
                uint32_t packet_size = data.at(1 + processed_bytes) + 6;
                if (data.size() + (bytes_received - processed_bytes) < packet_size) {
                  data.insert(data.end(), buffer.begin() + processed_bytes, buffer.begin() + processed_bytes + (bytes_received - processed_bytes));
                  processed_bytes += bytes_received;
                } else {
                  data.insert(data.end(), buffer.begin() + processed_bytes, buffer.begin() + processed_bytes + (packet_size - (int32_t)data.size()));
                  processed_bytes += packet_size - data.size();
                  ProcessPacket(data);
                  data.clear();
                }
              }
            }
          }
        }
      }
      catch (BaseLib::SocketClosedException &ex) {
        socket_->close();
        _out.printWarning("Warning: Connection to server closed.");
        continue;
      }
      catch (BaseLib::SocketTimeOutException &ex) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        continue;
      }
      catch (BaseLib::SocketOperationException &ex) {
        socket_->close();
        _out.printError("Error: " + std::string(ex.what()));
        continue;
      }
      catch (const std::exception &ex) {
        socket_->close();
        _out.printError("Error: " + std::string(ex.what()));
        continue;
      }
    }
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Tcp::ProcessPacket(const std::vector<uint8_t> &packet) {
  try {
    uint8_t packet_type = packet.at(0);

    std::unique_lock<std::mutex> requests_guard(requests_mutex_);
    auto request_iterator = requests_.find(packet_type);
    if (request_iterator != requests_.end()) {
      if (Gd::bl->debugLevel >= 4) _out.printInfo("Info: Processing packet as response: " + BaseLib::HelperFunctions::getHexString(packet));
      std::shared_ptr<Request> request = request_iterator->second;
      requests_guard.unlock();
      request->response = packet;
      {
        std::lock_guard<std::mutex> lock(request->mutex);
        request->mutex_ready = true;
      }
      request->condition_variable.notify_one();
      return;
    } else requests_guard.unlock();

    if (packet_type == 0xE5) {
      if (Gd::bl->debugLevel >= 4) _out.printInfo("Info: E5 packet received.");
      return;
    } else if(packet_type == 0x10) {
      if (Gd::bl->debugLevel >= 4) _out.printInfo("Info: 0x10 packet received: " + BaseLib::HelperFunctions::getHexString(packet));
      return;
    }

    PMbusPacket mbus_packet = std::make_shared<MbusPacket>(packet);
    if (mbus_packet->headerValid()) {
      raisePacketReceived(mbus_packet);
    } else _out.printWarning("Warning: Could not parse packet: " + BaseLib::HelperFunctions::getHexString(packet));
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

}