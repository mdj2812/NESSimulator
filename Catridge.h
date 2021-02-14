#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "Mapper_000.h"

class Catridge
{
public:
  Catridge(const std::string &sFileName);
  ~Catridge() {}

  enum MIRROR
  {
    HORIZONTAL,
    VERTICAL,
    ONESCREEN_LO,
    ONESCREEN_HI
  } mirror = HORIZONTAL;

  bool ImageValid() const { return bImageValid; }

  bool cpuRead(uint16_t addr, uint8_t &data) const
  {
    uint32_t mappedAddr = 0;
    if (pMapper->cpuMapRead(addr, mappedAddr))
    {
      data = vPRGMemory[mappedAddr];
      return true;
    }
    return false;
  }
  bool cpuWrite(uint16_t addr, uint8_t data)
  {
    uint32_t mappedAddr = 0;
    if (pMapper->cpuMapWrite(addr, mappedAddr))
    {
      vPRGMemory[mappedAddr] = data;
      return true;
    }
    return false;
  }

  bool ppuRead(uint16_t addr, uint8_t &data) const
  {
    uint32_t mappedAddr = 0;
    if (pMapper->ppuMapRead(addr, mappedAddr))
    {
      data = vCHRMemory[mappedAddr];
      return true;
    }
    return false;
  }
  bool ppuWrite(uint16_t addr, uint8_t data)
  {
    uint32_t mappedAddr = 0;
    if (pMapper->ppuMapWrite(addr, mappedAddr))
    {
      vCHRMemory[mappedAddr] = data;
      return true;
    }
    return false;
  }

private:
  bool bImageValid{false};

  std::vector<uint8_t> vPRGMemory;
  std::vector<uint8_t> vCHRMemory;

  std::shared_ptr<Mapper> pMapper;

  uint8_t nMapperID{0};
  uint8_t nPRGBanks{0};
  uint8_t nCHRBanks{0};
};