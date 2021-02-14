#include "Mapper.h"

/**
 * @brief Mapper 000
 *
 * Please refer to https://wiki.nesdev.com/w/index.php/NROM
 *
 */
class Mapper_000 : public Mapper
{
public:
  Mapper_000(uint8_t prgBanks, uint8_t chrBanks) : Mapper(prgBanks, chrBanks) {}
  ~Mapper_000() {}

  virtual bool cpuMapRead(uint16_t addr, uint32_t &mappedAddr)
  {
    if (addr >= 0x8000u && addr <= 0xFFFFu)
    {
      mappedAddr = addr & (nPRGBanks > 1 ? 0x7FFFu : 0x3FFFu);
      return true;
    }
    return false;
  }
  virtual bool cpuMapWrite(uint16_t addr, const uint32_t &mappedAddr)
  {
    if (addr >= 0x8000u && addr <= 0xFFFFu)
    {
      return true;
    }
    return false;
  }
  virtual bool ppuMapRead(uint16_t addr, uint32_t &mappedAddr)
  {
    if (addr >= 0x0000u && addr <= 0x1FFFu)
    {
      mappedAddr = addr;
      return true;
    }
    return false;
  }
  virtual bool ppuMapWrite(uint16_t addr, uint32_t &mappedAddr)
  {
    if (addr >= 0x0000u && addr <= 0x1FFFu)
    {
      if (nCHRBanks == 0)
      {
        mappedAddr = addr;
        return true;
      }
    }
    return false;
  }
};