#pragma once

constexpr uint16_t crc16(const char* data, const std::size_t len)
{
  const uint16_t poly = 0x8005;
  uint16_t crc16 = 0x0000U;
  for (std::size_t i = 0; i < len; ++i) {
    crc16 = crc16 ^ (data[i] << 8);
    for (std::size_t j = 0; j < 8; ++j) {
      if (crc16 & 0x8000)
        crc16 = (crc16 << 1) ^ poly;
      else
        crc16 <<= 1;
      crc16 &= 0xffff;
    }
  }
  return crc16;
}