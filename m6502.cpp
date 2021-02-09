#include "m6502.h"
#include "Bus.h"

using namespace std;

uint8_t m6502::read(uint16_t addr, bool bReadOnly) const {
  return bus->read(addr, false);
}

void m6502::write(uint16_t addr, uint8_t data) { bus->write(addr, data); }

std::map<uint16_t, std::string> m6502::disassemble(uint16_t nStart,
                                                   uint16_t nStop) {
  uint32_t addr = nStart;
  uint8_t value = 0x00u;
  uint8_t lo = 0x00u;
  uint8_t hi = 0x00u;
  map<uint16_t, string> mapLines;
  uint16_t lineAddr = 0;

  auto hex = [](uint32_t value, uint8_t numberOfFigure) {
    std::string s(numberOfFigure, '0');
    for (int i = numberOfFigure - 1; i >= 0; i--, value >>= 4)
      s[i] = "0123456789ABCDEF"[value & 0xF];
    return s;
  };

  while (addr <= (uint32_t)nStop) {
    lineAddr = addr;

    string sInst = "$" + hex(addr, 4) + ": ";
    auto opcode = bus->read(addr++, true);
    sInst += lookup[opcode].name + ' ';

    if (lookup[opcode].addrmode == &m6502::IMP) {
      sInst += " {IMP}";
    } else if (lookup[opcode].addrmode == &m6502::IMM) {
      value = bus->read(addr, true);
      addr++;
      sInst += "#$" + hex(value, 2) + " {IMM}";
    } else if (lookup[opcode].addrmode == &m6502::ZP0) {
      lo = bus->read(addr, true);
      addr++;
      hi = 0x00;
      sInst += "$" + hex(lo, 2) + " {ZP0}";
    } else if (lookup[opcode].addrmode == &m6502::ZPX) {
      lo = bus->read(addr, true);
      addr++;
      hi = 0x00;
      sInst += "$" + hex(lo, 2) + ", X {ZPX}";
    } else if (lookup[opcode].addrmode == &m6502::ZPY) {
      lo = bus->read(addr, true);
      addr++;
      hi = 0x00;
      sInst += "$" + hex(lo, 2) + ", Y {ZPY}";
    } else if (lookup[opcode].addrmode == &m6502::IZX) {
      lo = bus->read(addr, true);
      addr++;
      hi = 0x00;
      sInst += "($" + hex(lo, 2) + ", X) {IZX}";
    } else if (lookup[opcode].addrmode == &m6502::IZY) {
      lo = bus->read(addr, true);
      addr++;
      hi = 0x00;
      sInst += "($" + hex(lo, 2) + "), Y {IZY}";
    } else if (lookup[opcode].addrmode == &m6502::ABS) {
      lo = bus->read(addr, true);
      addr++;
      hi = bus->read(addr, true);
      addr++;
      sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + " {ABS}";
    } else if (lookup[opcode].addrmode == &m6502::ABX) {
      lo = bus->read(addr, true);
      addr++;
      hi = bus->read(addr, true);
      addr++;
      sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + ", X {ABX}";
    } else if (lookup[opcode].addrmode == &m6502::ABY) {
      lo = bus->read(addr, true);
      addr++;
      hi = bus->read(addr, true);
      addr++;
      sInst += "$" + hex((uint16_t)(hi << 8) | lo, 4) + ", Y {ABY}";
    } else if (lookup[opcode].addrmode == &m6502::IND) {
      lo = bus->read(addr, true);
      addr++;
      hi = bus->read(addr, true);
      addr++;
      sInst += "($" + hex((uint16_t)(hi << 8) | lo, 4) + ") {IND}";
    } else if (lookup[opcode].addrmode == &m6502::REL) {
      value = bus->read(addr, true);
      addr++;
      sInst += "$" + hex(value, 2) + " [$" + hex(addr + value, 4) + "] {REL}";
    }

    mapLines[lineAddr] = sInst;
  }

  return mapLines;
}
