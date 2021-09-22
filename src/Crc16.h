/* Copyright 2013-2019 Homegear GmbH */

#ifndef CRC16_H
#define CRC16_H

#include <cstdint>
#include <vector>
#include <map>

namespace Mbus {

class Crc16 {
 public:
  Crc16();
  virtual ~Crc16() {}
  uint16_t calculate(const std::vector<uint8_t> &data, int32_t offset);
 private:
  std::map<uint16_t, uint16_t> _crcTable;
  void initCrcTable();
};

}

#endif
