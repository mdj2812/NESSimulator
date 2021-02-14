#pragma once

#include <cstdint>

class Mapper
{
public:
  Mapper(uint8_t prgBanks, uint8_t chrBanks)
      : nPRGBanks(prgBanks), nCHRBanks(chrBanks) {}
  ~Mapper() {}

  virtual bool cpuMapRead(uint16_t addr, uint32_t &mappedAddr) = 0;
  virtual bool cpuMapWrite(uint16_t addr, const uint32_t &mappedAddr) = 0;
  virtual bool ppuMapRead(uint16_t addr, uint32_t &mappedAddr) = 0;
  virtual bool ppuMapWrite(uint16_t addr, uint32_t &mappedAddr) = 0;

protected:
  uint8_t nPRGBanks{0};
  uint8_t nCHRBanks{0};
};