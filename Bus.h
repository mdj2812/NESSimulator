#pragma once

#include <array>
#include <cstdint>

#include "m6502.h"

class Bus {
public:
  Bus() {
    ram.fill(0);
    cpu.ConnectBus(this);
  }
  ~Bus() {}

  uint8_t read(uint16_t addr, bool bReadOnly = false) const {
    if (addr >= 0x0000 && addr <= 0xFFFF) {
      return ram[addr];
    }
    return 0x00;
  }
  void write(uint16_t addr, uint8_t data) {
    if (addr >= 0x0000 && addr <= 0xFFFF) {
      ram[addr] = data;
    }
  }

  m6502 cpu;
  std::array<uint8_t, 64 * 1024> ram;

private:
};