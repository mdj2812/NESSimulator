#pragma once

#include <array>
#include <cstdint>

#include "Catridge.h"
#include "m2C02.h"
#include "m6502.h"

class Bus
{
public:
  Bus()
  {
    cpuRam.fill(0);
    cpu.ConnectBus(this);
  }
  ~Bus() {}

  uint8_t cpuRead(uint16_t addr, bool bReadOnly = false) const
  {
    uint8_t data = 0x00u;
    if (cart->cpuRead(addr, data))
    {
    }
    else if (addr >= 0x0000u && addr <= 0x1FFFu)
    {
      data = cpuRam[addr & 0x07FFu];
    }
    else if (addr >= 0x2000u && addr <= 0x3FFFu)
    {
      data = ppu.cpuRead(addr & 0x0007u, bReadOnly);
    }
    return data;
  }
  void cpuWrite(uint16_t addr, uint8_t data)
  {
    if (cart->cpuWrite(addr, data))
    {
    }
    else if (addr >= 0x0000u && addr <= 0x1FFFu)
    {
      cpuRam[addr & 0x07FFu] = data;
    }
    else if (addr >= 0x2000u && addr <= 0x3FFFu)
    {
      ppu.cpuWrite(addr & 0x0007u, data);
    }
  }

  void insertCartridge(const std::shared_ptr<Catridge> &catridge)
  {
    cart = catridge;
    ppu.ConnectCartridge(catridge);
  }
  void reset()
  {
    cpu.reset();
    nSystemClockCounter = 0;
  }
  void clock()
  {
    ppu.clock();
    // CPU clock is 3 time slower than PPU clock
    if (nSystemClockCounter % 3 == 0)
    {
      cpu.clock();
    }
    ++nSystemClockCounter;
  }

  m6502 cpu;
  m2C02 ppu;
  std::array<uint8_t, 2048> cpuRam;

private:
  uint32_t nSystemClockCounter{0};
  std::shared_ptr<Catridge> cart{nullptr};
};