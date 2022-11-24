/* Copyright 2013-2019 Homegear GmbH */

#include "Tcp.h"
#include "../Gd.h"

namespace Mbus {

Tcp::Tcp(std::shared_ptr<BaseLib::Systems::PhysicalInterfaceSettings> settings) : IMbusInterface(settings) {
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
    IPhysicalInterface::startListening();

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

    if (_listenThread.joinable()) _listenThread.join();
    listen_thread_ = std::thread(&Tcp::listen, this);
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Tcp::stopListening() {
  try {
    _stopped = true;
    _bl->threadManager.join(listen_thread_);
    IPhysicalInterface::stopListening();
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Tcp::Poll(const std::vector<uint8_t> &primary_addresses, const std::vector<int32_t> &secondary_addresses) {
  try {
    for (auto &address: primary_addresses) {
      std::vector<uint8_t> packet{0x10, 0x40, address, 0, 0x16};
      addCrc8(packet);

      RawSend(packet);

      for (uint32_t i = 0; i < 10; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (_stopped) return;
      }

      packet.at(1) = 0x7B;
      addCrc8(packet);

      RawSend(packet);

      for (uint32_t i = 0; i < 30; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (_stopped) return;
      }
    }

    for (auto &address: secondary_addresses) {
      std::vector<uint8_t> packet_1{0x10, 0x40, 0xFF, 0, 0x16};
      addCrc8(packet_1);

      RawSend(packet_1);

      for (uint32_t i = 0; i < 20; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (_stopped) return;
      }

      //Packet to temporarily set primary address for this device to 0xFD.
      std::vector<uint8_t> packet_2{0x68, 0x0B, 0x0B, 0x68, 0x73, 0xFD, 0x52, (uint8_t)address, (uint8_t)(address >> 8), (uint8_t)(address >> 16), (uint8_t)(address >> 24), 0xFF, 0xFF, 0xFF, 0xFF, 0, 0x16};
      addCrc8(packet_2);

      RawSend(packet_2);

      for (uint32_t i = 0; i < 20; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (_stopped) return;
      }

      std::vector<uint8_t> packet_3{0x10, 0x7B, 0xFD, 0, 0x16};
      addCrc8(packet_3);

      RawSend(packet_3);

      for (uint32_t i = 0; i < 30; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (_stopped) return;
      }
    }
  }
  catch (const std::exception &ex) {
    _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Tcp::RawSend(std::vector<uint8_t> &packet) {
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

void Tcp::listen() {
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

        bytes_received = socket_->proofread((char *)buffer.data(), buffer.size());

        if (BaseLib::HelperFunctions::getTime() - last_activity > 2000) data.clear();
        last_activity = BaseLib::HelperFunctions::getTime();

        if (bytes_received > 0) {
          if (Gd::bl->debugLevel >= 5) _out.printDebug("Debug: Raw packet received " + BaseLib::HelperFunctions::getHexString(buffer.data(), bytes_received));
          uint32_t processed_bytes = 0;
          while (processed_bytes < bytes_received) {
            if (data.empty()) {
              if (buffer.at(0 + processed_bytes) == 0xE5) {
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
                  data.insert(data.end(), buffer.begin() + processed_bytes, buffer.begin() + processed_bytes + (5 - data.size()));
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
                  data.insert(data.end(), buffer.begin() + processed_bytes, buffer.begin() + processed_bytes + (packet_size - data.size()));
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