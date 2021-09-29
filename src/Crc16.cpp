/* Copyright 2013-2019 Homegear GmbH */

#include <homegear-base/HelperFunctions/HelperFunctions.h>
#include "Crc16.h"

namespace Mbus {

Crc16::Crc16() {
  if (_crcTable.empty()) initCrcTable();
}

void Crc16::initCrcTable() {
  uint32_t bit, crc;

  for (uint32_t i = 0; i < 256; i++) {
    crc = i << 8;

    for (uint32_t j = 0; j < 8; j++) {

      bit = crc & 0x8000;
      crc <<= 1;
      if (bit) crc ^= 0x3d65;
    }

    crc &= 0xFFFF;
    _crcTable[i] = crc;
  }
}

uint16_t Crc16::calculate(const std::vector<uint8_t> &data, int32_t offset) {
  uint16_t crc = 0x0000;
  for (uint32_t i = offset; i < data.size(); i++) {
    crc = (crc << 8) ^ _crcTable[((crc >> 8) & 0xff) ^ data[i]];
  }

  return crc ^ 0xFFFF;
}
}
