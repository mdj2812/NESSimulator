#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

class Bus;

class m6502 {
public:
  m6502() {}
  ~m6502() { bus.release(); }

  // External event functions
  void clock() {
    if (cycles == 0) {
      opcode = read(pc++);
      SetFlag(U, true);

      cycles = lookup[opcode].cycles;

      auto additionalCycle1 = (this->*lookup[opcode].addrmode)();
      auto additionalCycle2 = (this->*lookup[opcode].operate)();

      cycles += (additionalCycle1 & additionalCycle2);
      SetFlag(U, true);
    }

    cycles--;
  }
  void reset() {
    a = x = y = 0;
    stackPointer = 0xFDu;
    status = 0x00 | U;

    addrAbs = 0xFFFC;
    uint16_t lo = read(addrAbs);
    uint16_t hi = read(addrAbs + 1);
    pc = (hi << 8) | lo;

    addrRel = 0x0000u;
    addrAbs = 0x0000u;
    fetched = 0x00u;

    cycles = 8;
  }
  void irq() {
    if (GetFlag(I) == 0) {
      write(0x0100u + stackPointer, (pc >> 8) & 0x00FFu);
      --stackPointer;
      write(0x0100u + stackPointer, pc & 0x00FFu);
      --stackPointer;

      SetFlag(B, 0);
      SetFlag(U, 1);
      SetFlag(I, 1);
      write(0x0100 + stackPointer, status);
      --stackPointer;

      addrAbs = 0xFFFEu;
      uint16_t lo = read(addrAbs);
      uint16_t hi = read(addrAbs + 1);
      pc = (hi << 8) | lo;

      cycles = 7;
    }
  }
  void nmi() {
    if (GetFlag(I) == 0) {
      write(0x0100u + stackPointer, (pc >> 8) & 0x00FFu);
      --stackPointer;
      write(0x0100u + stackPointer, pc & 0x00FFu);
      --stackPointer;

      SetFlag(B, 0);
      SetFlag(U, 1);
      SetFlag(I, 1);
      write(0x0100 + stackPointer, status);
      --stackPointer;

      addrAbs = 0xFFFAu;
      uint16_t lo = read(addrAbs);
      uint16_t hi = read(addrAbs + 1);
      pc = (hi << 8) | lo;

      cycles = 8;
    }
  }

  bool complete() { return cycles == 0; }

  void ConnectBus(Bus *busPtr) { bus.reset(busPtr); }

  enum FLAGS_6502 {
    C = (1 << 0), // Carry Flag
    Z = (1 << 1), // Zero Flag
    I = (1 << 2), // Interrupt Flag
    D = (1 << 3), // Decimal Flag
    B = (1 << 4), // Break Command
    U = (1 << 5), // Unused
    V = (1 << 6), // Overflow Command
    N = (1 << 7)  // Negative Command
  };

  // Addressing Modes
  uint8_t IMP() {
    fetched = a;
    return 0;
  }
  uint8_t IMM() {
    addrAbs = pc++;
    return 0;
  }
  uint8_t ZP0() {
    addrAbs = read(pc++);
    addrAbs &= 0x00FFu;
    return 0;
  }
  uint8_t ZPX() {
    addrAbs = read(pc++) + x;
    addrAbs &= 0x00FFu;
    return 0;
  }
  uint8_t ZPY() {
    addrAbs = read(pc++) + y;
    addrAbs &= 0x00FFu;
    return 0;
  }
  uint8_t REL() {
    addrRel = read(pc++);
    if (addrRel & 0x80) {
      addrRel |= 0xFF00;
    }
    return 0;
  }
  uint8_t ABS() {
    uint16_t lo = read(pc++);
    uint16_t hi = read(pc++);
    addrAbs = (hi << 8) | lo;
    return 0;
  }
  uint8_t ABX() {
    uint16_t lo = read(pc++);
    uint16_t hi = read(pc++);
    addrAbs = ((hi << 8) | lo) + x;
    // if page changed, need an additional clock cycle
    if ((addrAbs & 0xFF00u) != (hi << 8)) {
      return 1;
    }
    return 0;
  }
  uint8_t ABY() {
    uint16_t lo = read(pc++);
    uint16_t hi = read(pc++);
    addrAbs = ((hi << 8) | lo) + y;
    // if page changed, need an additional clock cycle
    if ((addrAbs & 0xFF00u) != (hi << 8)) {
      return 1;
    }
    return 0;
  }
  uint8_t IND() {
    uint16_t ptrLo = read(pc++);
    uint16_t ptrHi = read(pc++);
    uint16_t ptr = (ptrHi << 8) | ptrLo;
    // simulate page boundary hardware bug
    if (ptrLo == 0x00FFu) {
      // ignore this bug by remain in same page
      addrAbs = (read(ptr & 0xFF00u) << 8) | read(ptr);
    } else {
      addrAbs = (read(ptr + 1) << 8) | read(ptr);
    }
    return 0;
  }
  uint8_t IZX() {
    uint16_t t = read(pc++);
    uint16_t lo = read((uint16_t)(t + (uint16_t)x) & 0x00FFu);
    uint16_t hi = read((uint16_t)(t + (uint16_t)x + 1) & 0x00FFu);
    addrAbs = (hi << 8) | lo;
    return 0;
  }
  uint8_t IZY() {
    uint16_t t = read(pc++);
    uint16_t lo = read(t & 0x00FFu);
    uint16_t hi = read((t + 1) & 0x00FFu);
    addrAbs = ((hi << 8) | lo) + y;
    if ((addrAbs & 0xFF00u) != (hi << 8)) {
      return 1;
    }
    return 0;
  }

  // Opcodes
  /**
   * @brief Instruction Opcodes
   *
   * @return uint8_t additional cycles required
   */
  uint8_t LDA() {
    fetch();
    a = fetched;
    SetFlag(Z, a == 0x00u);
    SetFlag(N, a & 0x80u);
    return 1;
  }
  uint8_t LDX() {
    fetch();
    x = fetched;
    SetFlag(Z, x == 0x00u);
    SetFlag(N, x & 0x80u);
    return 1;
  }
  uint8_t LDY() {
    fetch();
    y = fetched;
    SetFlag(Z, y == 0x00u);
    SetFlag(N, y & 0x80u);
    return 1;
  }
  uint8_t STA() {
    write(addrAbs, a);
    return 0;
  }
  uint8_t STX() {
    write(addrAbs, x);
    return 0;
  }
  uint8_t STY() {
    write(addrAbs, y);
    return 0;
  }
  uint8_t TAX() {
    x = a;
    SetFlag(Z, x == 0x00u);
    SetFlag(N, x & 0x80u);
    return 0;
  }
  uint8_t TAY() {
    y = a;
    SetFlag(Z, y == 0x00u);
    SetFlag(N, y & 0x80u);
    return 0;
  }
  uint8_t TXA() {
    a = x;
    SetFlag(Z, a == 0x00u);
    SetFlag(N, a & 0x80u);
    return 0;
  }
  uint8_t TYA() {
    a = y;
    SetFlag(Z, a == 0x00u);
    SetFlag(N, a & 0x80u);
    return 0;
  }
  uint8_t TSX() {
    x = stackPointer;
    SetFlag(Z, x == 0x00u);
    SetFlag(N, x & 0x80u);
    return 0;
  }
  uint8_t TXS() {
    stackPointer = x;
    return 0;
  }
  uint8_t PHA() {
    write(0x0100u + stackPointer, a);
    --stackPointer;
    return 0;
  }
  uint8_t PHP() {
    write(0x0100u + stackPointer, status | B | U);
    SetFlag(B, 0);
    SetFlag(U, 0);
    --stackPointer;
    return 0;
  }
  uint8_t PLA() {
    ++stackPointer;
    a = read(0x0100 + stackPointer);
    SetFlag(Z, a == 0x00u);
    SetFlag(N, a & 0x80u);
    return 0;
  }
  uint8_t PLP() {
    ++stackPointer;
    status = read(0x0100 + stackPointer);
    SetFlag(U, 1);
    return 0;
  }
  uint8_t AND() {
    fetch();
    a &= fetched;
    SetFlag(Z, a == 0x00u);
    SetFlag(N, a & 0x80u);
    return 1;
  }
  uint8_t EOR() {
    fetch();
    a ^= fetched;
    SetFlag(Z, a == 0x00u);
    SetFlag(N, a & 0x80u);
    return 1;
  }
  uint8_t ORA() {
    fetch();
    a |= fetched;
    SetFlag(Z, a == 0x00u);
    SetFlag(N, a & 0x80u);
    return 1;
  }
  uint8_t BIT() {
    fetch();
    uint16_t tmp = a & fetched;
    SetFlag(Z, (tmp & 0x00FFu) == 0);
    SetFlag(N, tmp & 0x80u);
    SetFlag(V, fetched & (1 << 6));
    return 0;
  }
  uint8_t ADC() {
    fetch();
    uint16_t tmp = (uint16_t)a + (uint16_t)fetched + (uint16_t)GetFlag(C);
    SetFlag(C, tmp > 255);
    SetFlag(Z, (tmp & 0x00FFu) == 0);
    SetFlag(N, tmp & 0x80u);
    SetFlag(V, (~((uint16_t)a ^ (uint16_t)fetched) &
                ((uint16_t)a ^ (uint16_t)tmp)) &
                   0x0080u);
    a = tmp & 0x00FF;
    return 1;
  }
  uint8_t SBC() {
    fetch();
    uint16_t value = (uint16_t)fetched ^ 0x00FFu;
    uint16_t tmp = (uint16_t)a + value + (uint16_t)GetFlag(C);
    SetFlag(C, tmp > 255);
    SetFlag(Z, (tmp & 0x00FFu) == 0);
    SetFlag(N, tmp & 0x80u);
    SetFlag(V, (~((uint16_t)a ^ (uint16_t)fetched) &
                ((uint16_t)a ^ (uint16_t)tmp)) &
                   0x0080u);
    a = tmp & 0x00FF;
    return 1;
  }
  uint8_t CMP() {
    fetch();
    uint16_t tmp = (uint16_t)a - (uint16_t)fetched;
    SetFlag(C, a >= fetched);
    SetFlag(Z, (tmp & 0x00FFu) == 0);
    SetFlag(N, tmp & 0x80u);
    return 1;
  }
  uint8_t CPX() {
    fetch();
    uint16_t tmp = (uint16_t)x - (uint16_t)fetched;
    SetFlag(C, x >= fetched);
    SetFlag(Z, (tmp & 0x00FFu) == 0);
    SetFlag(N, tmp & 0x80u);
    return 0;
  }
  uint8_t CPY() {
    fetch();
    uint16_t tmp = (uint16_t)y - (uint16_t)fetched;
    SetFlag(C, y >= fetched);
    SetFlag(Z, (tmp & 0x00FFu) == 0);
    SetFlag(N, tmp & 0x80u);
    return 0;
  }
  uint8_t INC() {
    fetch();
    uint16_t tmp = fetched + 1;
    write(addrAbs, tmp & 0x00FFu);
    SetFlag(Z, (tmp & 0x00FFu) == 0);
    SetFlag(N, tmp & 0x80u);
    return 0;
  }
  uint8_t INX() {
    ++x;
    SetFlag(Z, (x & 0x00FFu) == 0);
    SetFlag(N, x & 0x80u);
    return 0;
  }
  uint8_t INY() {
    ++y;
    SetFlag(Z, (y & 0x00FFu) == 0);
    SetFlag(N, y & 0x80u);
    return 0;
  }
  uint8_t DEC() {
    fetch();
    uint16_t tmp = fetched - 1;
    write(addrAbs, tmp & 0x00FFu);
    SetFlag(Z, (tmp & 0x00FFu) == 0);
    SetFlag(N, tmp & 0x80u);
    return 0;
  }
  uint8_t DEX() {
    --x;
    SetFlag(Z, x == 0);
    SetFlag(N, x & 0x80u);
    return 0;
  }
  uint8_t DEY() {
    --y;
    SetFlag(Z, y == 0);
    SetFlag(N, y & 0x80u);
    return 0;
  }
  uint8_t ASL() {
    fetch();
    uint16_t tmp = (uint16_t)fetched << 1;
    SetFlag(C, tmp & 0xFF00u);
    SetFlag(Z, (tmp & 0x00FFu) == 0);
    SetFlag(N, tmp & 0x80u);
    if (lookup[opcode].addrmode == &m6502::IMP) {
      a = tmp & 0x00FFu;
    } else {
      write(addrAbs, tmp & 0x00FFu);
    }
    return 0;
  }
  uint8_t LSR() {
    fetch();
    SetFlag(C, fetched & 0x0001u);
    uint16_t tmp = fetched >> 1;
    SetFlag(Z, (tmp & 0x00FFu) == 0);
    SetFlag(N, tmp & 0x80u);
    if (lookup[opcode].addrmode == &m6502::IMP) {
      a = tmp & 0x00FFu;
    } else {
      write(addrAbs, tmp & 0x00FFu);
    }
    return 0;
  }
  uint8_t ROL() {
    fetch();
    uint16_t tmp = (uint16_t)(fetched << 1) | GetFlag(C);
    SetFlag(C, tmp & 0xFF00u);
    SetFlag(Z, (tmp & 0x00FFu) == 0);
    SetFlag(N, tmp & 0x80u);
    if (lookup[opcode].addrmode == &m6502::IMP) {
      a = tmp & 0x00FFu;
    } else {
      write(addrAbs, tmp & 0x00FFu);
    }
    return 0;
  }
  uint8_t ROR() {
    fetch();
    uint16_t tmp = (uint16_t)(GetFlag(C) << 7) | (fetched >> 1);
    SetFlag(C, fetched & 0x01u);
    SetFlag(Z, (tmp & 0x00FFu) == 0);
    SetFlag(N, tmp & 0x80u);
    if (lookup[opcode].addrmode == &m6502::IMP) {
      a = tmp & 0x00FFu;
    } else {
      write(addrAbs, tmp & 0x00FFu);
    }
    return 0;
  }
  uint8_t JMP() {
    pc = addrAbs;
    return 0;
  }
  uint8_t JSR() {
    --pc;
    write(0x0100u + stackPointer, (pc >> 8) & 0x00FFu);
    --stackPointer;
    write(0x0100u + stackPointer, pc & 0x00FFu);
    --stackPointer;
    pc = addrAbs;
    return 0;
  }
  uint8_t RTS() {
    ++stackPointer;
    pc = read(0x0100u + stackPointer);
    ++stackPointer;
    pc |= (uint16_t)read(0x0100u + stackPointer) << 8;

    ++pc;
    return 0;
  }
  uint8_t BCC() {
    if (GetFlag(C) == 0) {
      ++cycles;
      addrAbs = pc + addrRel;
      if ((addrAbs & 0xFF00u) != (pc & 0xFF00u)) {
        ++cycles;
      }
      pc = addrAbs;
    }
    return 0;
  }
  uint8_t BCS() {
    if (GetFlag(C) == 1) {
      ++cycles;
      addrAbs = pc + addrRel;
      if ((addrAbs & 0xFF00u) != (pc & 0xFF00u)) {
        ++cycles;
      }
      pc = addrAbs;
    }
    return 0;
  }
  uint8_t BEQ() {
    if (GetFlag(Z) == 1) {
      ++cycles;
      addrAbs = pc + addrRel;
      if ((addrAbs & 0xFF00u) != (pc & 0xFF00u)) {
        ++cycles;
      }
      pc = addrAbs;
    }
    return 0;
  }
  uint8_t BMI() {
    if (GetFlag(N) == 1) {
      ++cycles;
      addrAbs = pc + addrRel;
      if ((addrAbs & 0xFF00u) != (pc & 0xFF00u)) {
        ++cycles;
      }
      pc = addrAbs;
    }
    return 0;
  }
  uint8_t BNE() {
    if (GetFlag(Z) == 0) {
      ++cycles;
      addrAbs = pc + addrRel;
      if ((addrAbs & 0xFF00u) != (pc & 0xFF00u)) {
        ++cycles;
      }
      pc = addrAbs;
    }
    return 0;
  }
  uint8_t BPL() {
    if (GetFlag(N) == 0) {
      ++cycles;
      addrAbs = pc + addrRel;
      if ((addrAbs & 0xFF00u) != (pc & 0xFF00u)) {
        ++cycles;
      }
      pc = addrAbs;
    }
    return 0;
  }
  uint8_t BVC() {
    if (GetFlag(V) == 0) {
      ++cycles;
      addrAbs = pc + addrRel;
      if ((addrAbs & 0xFF00u) != (pc & 0xFF00u)) {
        ++cycles;
      }
      pc = addrAbs;
    }
    return 0;
  }
  uint8_t BVS() {
    if (GetFlag(V) == 1) {
      ++cycles;
      addrAbs = pc + addrRel;
      if ((addrAbs & 0xFF00u) != (pc & 0xFF00u)) {
        ++cycles;
      }
      pc = addrAbs;
    }
    return 0;
  }
  uint8_t CLC() {
    SetFlag(C, false);
    return 0;
  }
  uint8_t CLD() {
    SetFlag(D, false);
    return 0;
  }
  uint8_t CLI() {
    SetFlag(I, false);
    return 0;
  }
  uint8_t CLV() {
    SetFlag(V, false);
    return 0;
  }
  uint8_t SEC() {
    SetFlag(C, true);
    return 0;
  }
  uint8_t SED() {
    SetFlag(D, true);
    return 0;
  }
  uint8_t SEI() {
    SetFlag(I, true);
    return 0;
  }
  uint8_t BRK() {
    pc++;

    SetFlag(I, 1);
    write(0x0100 + stackPointer, (pc >> 8) & 0x00FF);
    --stackPointer;
    write(0x0100 + stackPointer, pc & 0x00FF);
    --stackPointer;

    SetFlag(B, 1);
    write(0x0100 + stackPointer, status);
    --stackPointer;
    SetFlag(B, 0);

    pc = (uint16_t)read(0xFFFE) | ((uint16_t)read(0xFFFF) << 8);
    return 0;
  }
  uint8_t NOP() {
    // TODO: Not completed
    switch (opcode) {
    case 0x1C:
    case 0x3C:
    case 0x5C:
    case 0x7C:
    case 0xDC:
    case 0xFC:
      return 1;
      break;
    }
    return 0;
  }
  uint8_t RTI() {
    ++stackPointer;
    status = read(0x0100u + stackPointer);
    status &= ~B;
    status &= ~U;

    ++stackPointer;
    pc = read(0x0100 + stackPointer);
    ++stackPointer;
    pc |= (read(0x0100 + stackPointer) << 8);
    return 0;
  }

  // illegal opcodes
  uint8_t XXX() { return 0; }

  /**
   * @brief This is the disassembly function.
   *
   * Its workings are not required for emulation. It is merely a convenience
   * function to turn the binary instruction code into human readable form. It's
   * included as part of teh emulator because it can take advantage of many of
   * the CPUs internal operations to do this.
   *
   * @param nStart start address to read an instruction byte
   * @param nStop stop address for disassemble
   * @return std::map<uint16_t, std::string> instruction's address as the key,
   * formed string as value. This makes it convenient to look for later as the
   * instructions are variable in length
   */
  std::map<uint16_t, std::string> disassemble(uint16_t nStart, uint16_t nStop);

  uint8_t a{0x00u};            // Accumulator Register
  uint8_t x{0x00u};            // X Register
  uint8_t y{0x00u};            // Y Register
  uint8_t stackPointer{0x00u}; // Stack Pointer
  uint16_t pc{0x0000u};        // Program Counter
  uint8_t status{0x00u};       // Status Register

private:
  std::unique_ptr<Bus> bus{nullptr};

  uint8_t fetch() {
    if (lookup[opcode].addrmode != &m6502::IMP) {
      fetched = read(addrAbs);
    }
    return fetched;
  }
  uint8_t fetched{0x00u};

  uint16_t addrAbs{0x0000u};
  uint16_t addrRel{0x0000u};
  uint8_t opcode{0x00};
  uint8_t cycles{0x00};

  uint8_t read(uint16_t addr, bool bReadOnly = false) const;
  void write(uint16_t addr, uint8_t data);

  // Access status register
  uint8_t GetFlag(FLAGS_6502 flag) { return ((status & flag) > 0) ? 1 : 0; }
  void SetFlag(FLAGS_6502 flag, bool v) {
    if (v) {
      status |= flag;
    } else {
      status &= ~flag;
    }
  }

  struct INSTRUCTION {
    std::string name;
    uint8_t (m6502::*operate)(void){nullptr};
    uint8_t (m6502::*addrmode)(void){nullptr};
    uint8_t cycles{0};
  };

  const std::vector<INSTRUCTION> lookup{{"BRK", &m6502::BRK, &m6502::IMM, 7},
                                        {"ORA", &m6502::ORA, &m6502::IZX, 6},
                                        {"???", &m6502::XXX, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 8},
                                        {"???", &m6502::NOP, &m6502::IMP, 3},
                                        {"ORA", &m6502::ORA, &m6502::ZP0, 3},
                                        {"ASL", &m6502::ASL, &m6502::ZP0, 5},
                                        {"???", &m6502::XXX, &m6502::IMP, 5},
                                        {"PHP", &m6502::PHP, &m6502::IMP, 3},
                                        {"ORA", &m6502::ORA, &m6502::IMM, 2},
                                        {"ASL", &m6502::ASL, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 2},
                                        {"???", &m6502::NOP, &m6502::IMP, 4},
                                        {"ORA", &m6502::ORA, &m6502::ABS, 4},
                                        {"ASL", &m6502::ASL, &m6502::ABS, 6},
                                        {"???", &m6502::XXX, &m6502::IMP, 6},
                                        {"BPL", &m6502::BPL, &m6502::REL, 2},
                                        {"ORA", &m6502::ORA, &m6502::IZY, 5},
                                        {"???", &m6502::XXX, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 8},
                                        {"???", &m6502::NOP, &m6502::IMP, 4},
                                        {"ORA", &m6502::ORA, &m6502::ZPX, 4},
                                        {"ASL", &m6502::ASL, &m6502::ZPX, 6},
                                        {"???", &m6502::XXX, &m6502::IMP, 6},
                                        {"CLC", &m6502::CLC, &m6502::IMP, 2},
                                        {"ORA", &m6502::ORA, &m6502::ABY, 4},
                                        {"???", &m6502::NOP, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 7},
                                        {"???", &m6502::NOP, &m6502::IMP, 4},
                                        {"ORA", &m6502::ORA, &m6502::ABX, 4},
                                        {"ASL", &m6502::ASL, &m6502::ABX, 7},
                                        {"???", &m6502::XXX, &m6502::IMP, 7},
                                        {"JSR", &m6502::JSR, &m6502::ABS, 6},
                                        {"AND", &m6502::AND, &m6502::IZX, 6},
                                        {"???", &m6502::XXX, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 8},
                                        {"BIT", &m6502::BIT, &m6502::ZP0, 3},
                                        {"AND", &m6502::AND, &m6502::ZP0, 3},
                                        {"ROL", &m6502::ROL, &m6502::ZP0, 5},
                                        {"???", &m6502::XXX, &m6502::IMP, 5},
                                        {"PLP", &m6502::PLP, &m6502::IMP, 4},
                                        {"AND", &m6502::AND, &m6502::IMM, 2},
                                        {"ROL", &m6502::ROL, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 2},
                                        {"BIT", &m6502::BIT, &m6502::ABS, 4},
                                        {"AND", &m6502::AND, &m6502::ABS, 4},
                                        {"ROL", &m6502::ROL, &m6502::ABS, 6},
                                        {"???", &m6502::XXX, &m6502::IMP, 6},
                                        {"BMI", &m6502::BMI, &m6502::REL, 2},
                                        {"AND", &m6502::AND, &m6502::IZY, 5},
                                        {"???", &m6502::XXX, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 8},
                                        {"???", &m6502::NOP, &m6502::IMP, 4},
                                        {"AND", &m6502::AND, &m6502::ZPX, 4},
                                        {"ROL", &m6502::ROL, &m6502::ZPX, 6},
                                        {"???", &m6502::XXX, &m6502::IMP, 6},
                                        {"SEC", &m6502::SEC, &m6502::IMP, 2},
                                        {"AND", &m6502::AND, &m6502::ABY, 4},
                                        {"???", &m6502::NOP, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 7},
                                        {"???", &m6502::NOP, &m6502::IMP, 4},
                                        {"AND", &m6502::AND, &m6502::ABX, 4},
                                        {"ROL", &m6502::ROL, &m6502::ABX, 7},
                                        {"???", &m6502::XXX, &m6502::IMP, 7},
                                        {"RTI", &m6502::RTI, &m6502::IMP, 6},
                                        {"EOR", &m6502::EOR, &m6502::IZX, 6},
                                        {"???", &m6502::XXX, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 8},
                                        {"???", &m6502::NOP, &m6502::IMP, 3},
                                        {"EOR", &m6502::EOR, &m6502::ZP0, 3},
                                        {"LSR", &m6502::LSR, &m6502::ZP0, 5},
                                        {"???", &m6502::XXX, &m6502::IMP, 5},
                                        {"PHA", &m6502::PHA, &m6502::IMP, 3},
                                        {"EOR", &m6502::EOR, &m6502::IMM, 2},
                                        {"LSR", &m6502::LSR, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 2},
                                        {"JMP", &m6502::JMP, &m6502::ABS, 3},
                                        {"EOR", &m6502::EOR, &m6502::ABS, 4},
                                        {"LSR", &m6502::LSR, &m6502::ABS, 6},
                                        {"???", &m6502::XXX, &m6502::IMP, 6},
                                        {"BVC", &m6502::BVC, &m6502::REL, 2},
                                        {"EOR", &m6502::EOR, &m6502::IZY, 5},
                                        {"???", &m6502::XXX, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 8},
                                        {"???", &m6502::NOP, &m6502::IMP, 4},
                                        {"EOR", &m6502::EOR, &m6502::ZPX, 4},
                                        {"LSR", &m6502::LSR, &m6502::ZPX, 6},
                                        {"???", &m6502::XXX, &m6502::IMP, 6},
                                        {"CLI", &m6502::CLI, &m6502::IMP, 2},
                                        {"EOR", &m6502::EOR, &m6502::ABY, 4},
                                        {"???", &m6502::NOP, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 7},
                                        {"???", &m6502::NOP, &m6502::IMP, 4},
                                        {"EOR", &m6502::EOR, &m6502::ABX, 4},
                                        {"LSR", &m6502::LSR, &m6502::ABX, 7},
                                        {"???", &m6502::XXX, &m6502::IMP, 7},
                                        {"RTS", &m6502::RTS, &m6502::IMP, 6},
                                        {"ADC", &m6502::ADC, &m6502::IZX, 6},
                                        {"???", &m6502::XXX, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 8},
                                        {"???", &m6502::NOP, &m6502::IMP, 3},
                                        {"ADC", &m6502::ADC, &m6502::ZP0, 3},
                                        {"ROR", &m6502::ROR, &m6502::ZP0, 5},
                                        {"???", &m6502::XXX, &m6502::IMP, 5},
                                        {"PLA", &m6502::PLA, &m6502::IMP, 4},
                                        {"ADC", &m6502::ADC, &m6502::IMM, 2},
                                        {"ROR", &m6502::ROR, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 2},
                                        {"JMP", &m6502::JMP, &m6502::IND, 5},
                                        {"ADC", &m6502::ADC, &m6502::ABS, 4},
                                        {"ROR", &m6502::ROR, &m6502::ABS, 6},
                                        {"???", &m6502::XXX, &m6502::IMP, 6},
                                        {"BVS", &m6502::BVS, &m6502::REL, 2},
                                        {"ADC", &m6502::ADC, &m6502::IZY, 5},
                                        {"???", &m6502::XXX, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 8},
                                        {"???", &m6502::NOP, &m6502::IMP, 4},
                                        {"ADC", &m6502::ADC, &m6502::ZPX, 4},
                                        {"ROR", &m6502::ROR, &m6502::ZPX, 6},
                                        {"???", &m6502::XXX, &m6502::IMP, 6},
                                        {"SEI", &m6502::SEI, &m6502::IMP, 2},
                                        {"ADC", &m6502::ADC, &m6502::ABY, 4},
                                        {"???", &m6502::NOP, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 7},
                                        {"???", &m6502::NOP, &m6502::IMP, 4},
                                        {"ADC", &m6502::ADC, &m6502::ABX, 4},
                                        {"ROR", &m6502::ROR, &m6502::ABX, 7},
                                        {"???", &m6502::XXX, &m6502::IMP, 7},
                                        {"???", &m6502::NOP, &m6502::IMP, 2},
                                        {"STA", &m6502::STA, &m6502::IZX, 6},
                                        {"???", &m6502::NOP, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 6},
                                        {"STY", &m6502::STY, &m6502::ZP0, 3},
                                        {"STA", &m6502::STA, &m6502::ZP0, 3},
                                        {"STX", &m6502::STX, &m6502::ZP0, 3},
                                        {"???", &m6502::XXX, &m6502::IMP, 3},
                                        {"DEY", &m6502::DEY, &m6502::IMP, 2},
                                        {"???", &m6502::NOP, &m6502::IMP, 2},
                                        {"TXA", &m6502::TXA, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 2},
                                        {"STY", &m6502::STY, &m6502::ABS, 4},
                                        {"STA", &m6502::STA, &m6502::ABS, 4},
                                        {"STX", &m6502::STX, &m6502::ABS, 4},
                                        {"???", &m6502::XXX, &m6502::IMP, 4},
                                        {"BCC", &m6502::BCC, &m6502::REL, 2},
                                        {"STA", &m6502::STA, &m6502::IZY, 6},
                                        {"???", &m6502::XXX, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 6},
                                        {"STY", &m6502::STY, &m6502::ZPX, 4},
                                        {"STA", &m6502::STA, &m6502::ZPX, 4},
                                        {"STX", &m6502::STX, &m6502::ZPY, 4},
                                        {"???", &m6502::XXX, &m6502::IMP, 4},
                                        {"TYA", &m6502::TYA, &m6502::IMP, 2},
                                        {"STA", &m6502::STA, &m6502::ABY, 5},
                                        {"TXS", &m6502::TXS, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 5},
                                        {"???", &m6502::NOP, &m6502::IMP, 5},
                                        {"STA", &m6502::STA, &m6502::ABX, 5},
                                        {"???", &m6502::XXX, &m6502::IMP, 5},
                                        {"???", &m6502::XXX, &m6502::IMP, 5},
                                        {"LDY", &m6502::LDY, &m6502::IMM, 2},
                                        {"LDA", &m6502::LDA, &m6502::IZX, 6},
                                        {"LDX", &m6502::LDX, &m6502::IMM, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 6},
                                        {"LDY", &m6502::LDY, &m6502::ZP0, 3},
                                        {"LDA", &m6502::LDA, &m6502::ZP0, 3},
                                        {"LDX", &m6502::LDX, &m6502::ZP0, 3},
                                        {"???", &m6502::XXX, &m6502::IMP, 3},
                                        {"TAY", &m6502::TAY, &m6502::IMP, 2},
                                        {"LDA", &m6502::LDA, &m6502::IMM, 2},
                                        {"TAX", &m6502::TAX, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 2},
                                        {"LDY", &m6502::LDY, &m6502::ABS, 4},
                                        {"LDA", &m6502::LDA, &m6502::ABS, 4},
                                        {"LDX", &m6502::LDX, &m6502::ABS, 4},
                                        {"???", &m6502::XXX, &m6502::IMP, 4},
                                        {"BCS", &m6502::BCS, &m6502::REL, 2},
                                        {"LDA", &m6502::LDA, &m6502::IZY, 5},
                                        {"???", &m6502::XXX, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 5},
                                        {"LDY", &m6502::LDY, &m6502::ZPX, 4},
                                        {"LDA", &m6502::LDA, &m6502::ZPX, 4},
                                        {"LDX", &m6502::LDX, &m6502::ZPY, 4},
                                        {"???", &m6502::XXX, &m6502::IMP, 4},
                                        {"CLV", &m6502::CLV, &m6502::IMP, 2},
                                        {"LDA", &m6502::LDA, &m6502::ABY, 4},
                                        {"TSX", &m6502::TSX, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 4},
                                        {"LDY", &m6502::LDY, &m6502::ABX, 4},
                                        {"LDA", &m6502::LDA, &m6502::ABX, 4},
                                        {"LDX", &m6502::LDX, &m6502::ABY, 4},
                                        {"???", &m6502::XXX, &m6502::IMP, 4},
                                        {"CPY", &m6502::CPY, &m6502::IMM, 2},
                                        {"CMP", &m6502::CMP, &m6502::IZX, 6},
                                        {"???", &m6502::NOP, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 8},
                                        {"CPY", &m6502::CPY, &m6502::ZP0, 3},
                                        {"CMP", &m6502::CMP, &m6502::ZP0, 3},
                                        {"DEC", &m6502::DEC, &m6502::ZP0, 5},
                                        {"???", &m6502::XXX, &m6502::IMP, 5},
                                        {"INY", &m6502::INY, &m6502::IMP, 2},
                                        {"CMP", &m6502::CMP, &m6502::IMM, 2},
                                        {"DEX", &m6502::DEX, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 2},
                                        {"CPY", &m6502::CPY, &m6502::ABS, 4},
                                        {"CMP", &m6502::CMP, &m6502::ABS, 4},
                                        {"DEC", &m6502::DEC, &m6502::ABS, 6},
                                        {"???", &m6502::XXX, &m6502::IMP, 6},
                                        {"BNE", &m6502::BNE, &m6502::REL, 2},
                                        {"CMP", &m6502::CMP, &m6502::IZY, 5},
                                        {"???", &m6502::XXX, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 8},
                                        {"???", &m6502::NOP, &m6502::IMP, 4},
                                        {"CMP", &m6502::CMP, &m6502::ZPX, 4},
                                        {"DEC", &m6502::DEC, &m6502::ZPX, 6},
                                        {"???", &m6502::XXX, &m6502::IMP, 6},
                                        {"CLD", &m6502::CLD, &m6502::IMP, 2},
                                        {"CMP", &m6502::CMP, &m6502::ABY, 4},
                                        {"NOP", &m6502::NOP, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 7},
                                        {"???", &m6502::NOP, &m6502::IMP, 4},
                                        {"CMP", &m6502::CMP, &m6502::ABX, 4},
                                        {"DEC", &m6502::DEC, &m6502::ABX, 7},
                                        {"???", &m6502::XXX, &m6502::IMP, 7},
                                        {"CPX", &m6502::CPX, &m6502::IMM, 2},
                                        {"SBC", &m6502::SBC, &m6502::IZX, 6},
                                        {"???", &m6502::NOP, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 8},
                                        {"CPX", &m6502::CPX, &m6502::ZP0, 3},
                                        {"SBC", &m6502::SBC, &m6502::ZP0, 3},
                                        {"INC", &m6502::INC, &m6502::ZP0, 5},
                                        {"???", &m6502::XXX, &m6502::IMP, 5},
                                        {"INX", &m6502::INX, &m6502::IMP, 2},
                                        {"SBC", &m6502::SBC, &m6502::IMM, 2},
                                        {"NOP", &m6502::NOP, &m6502::IMP, 2},
                                        {"???", &m6502::SBC, &m6502::IMP, 2},
                                        {"CPX", &m6502::CPX, &m6502::ABS, 4},
                                        {"SBC", &m6502::SBC, &m6502::ABS, 4},
                                        {"INC", &m6502::INC, &m6502::ABS, 6},
                                        {"???", &m6502::XXX, &m6502::IMP, 6},
                                        {"BEQ", &m6502::BEQ, &m6502::REL, 2},
                                        {"SBC", &m6502::SBC, &m6502::IZY, 5},
                                        {"???", &m6502::XXX, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 8},
                                        {"???", &m6502::NOP, &m6502::IMP, 4},
                                        {"SBC", &m6502::SBC, &m6502::ZPX, 4},
                                        {"INC", &m6502::INC, &m6502::ZPX, 6},
                                        {"???", &m6502::XXX, &m6502::IMP, 6},
                                        {"SED", &m6502::SED, &m6502::IMP, 2},
                                        {"SBC", &m6502::SBC, &m6502::ABY, 4},
                                        {"NOP", &m6502::NOP, &m6502::IMP, 2},
                                        {"???", &m6502::XXX, &m6502::IMP, 7},
                                        {"???", &m6502::NOP, &m6502::IMP, 4},
                                        {"SBC", &m6502::SBC, &m6502::ABX, 4},
                                        {"INC", &m6502::INC, &m6502::ABX, 7},
                                        {"???", &m6502::XXX, &m6502::IMP, 7}};
};