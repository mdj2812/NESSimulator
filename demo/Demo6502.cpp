#include "Bus.h"
#include "m6502.h"

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

using namespace std;

class DemoM6502 : public olc::PixelGameEngine {
public:
  DemoM6502() { sAppName = "m6502 Demonstration"; }

  Bus nes;
  map<uint16_t, string> mapAsm;

  string hex(uint32_t n, uint8_t d) {
    string s(d, '0');
    for (int i = d - 1; i >= 0; i--, n >>= 4)
      s[i] = "0123456789ABCDEF"[n & 0xF];
    return s;
  };

  void DrawRam(int x, int y, uint16_t nAddr, int nRows, int nColumns) {
    int nRamX = x, nRamY = y;
    for (int row = 0; row < nRows; row++) {
      string sOffset = "$" + hex(nAddr, 4) + ":";
      for (int col = 0; col < nColumns; col++) {
        sOffset += " " + hex(nes.read(nAddr, true), 2);
        nAddr += 1;
      }
      DrawString(nRamX, nRamY, sOffset);
      nRamY += 10;
    }
  }

  void DrawCpu(int x, int y) {
    std::string status = "STATUS: ";
    DrawString(x, y, "STATUS:", olc::WHITE);
    DrawString(x + 64, y, "N",
               nes.cpu.status & m6502::N ? olc::GREEN : olc::RED);
    DrawString(x + 80, y, "V",
               nes.cpu.status & m6502::V ? olc::GREEN : olc::RED);
    DrawString(x + 96, y, "-",
               nes.cpu.status & m6502::U ? olc::GREEN : olc::RED);
    DrawString(x + 112, y, "B",
               nes.cpu.status & m6502::B ? olc::GREEN : olc::RED);
    DrawString(x + 128, y, "D",
               nes.cpu.status & m6502::D ? olc::GREEN : olc::RED);
    DrawString(x + 144, y, "I",
               nes.cpu.status & m6502::I ? olc::GREEN : olc::RED);
    DrawString(x + 160, y, "Z",
               nes.cpu.status & m6502::Z ? olc::GREEN : olc::RED);
    DrawString(x + 178, y, "C",
               nes.cpu.status & m6502::C ? olc::GREEN : olc::RED);
    DrawString(x, y + 10, "PC: $" + hex(nes.cpu.pc, 4));
    DrawString(x, y + 20,
               "A: $" + hex(nes.cpu.a, 2) + "  [" + std::to_string(nes.cpu.a) +
                   "]");
    DrawString(x, y + 30,
               "X: $" + hex(nes.cpu.x, 2) + "  [" + std::to_string(nes.cpu.x) +
                   "]");
    DrawString(x, y + 40,
               "Y: $" + hex(nes.cpu.y, 2) + "  [" + std::to_string(nes.cpu.y) +
                   "]");
    DrawString(x, y + 50, "Stack P: $" + hex(nes.cpu.stackPointer, 4));
  }

  void DrawCode(int x, int y, int nLines) {
    auto it_a = mapAsm.find(nes.cpu.pc);
    int nLineY = (nLines >> 1) * 10 + y;
    if (it_a != mapAsm.end()) {
      DrawString(x, nLineY, (*it_a).second, olc::CYAN);
      while (nLineY < (nLines * 10) + y) {
        nLineY += 10;
        if (++it_a != mapAsm.end()) {
          DrawString(x, nLineY, (*it_a).second);
        }
      }
    }

    it_a = mapAsm.find(nes.cpu.pc);
    nLineY = (nLines >> 1) * 10 + y;
    if (it_a != mapAsm.end()) {
      while (nLineY > y) {
        nLineY -= 10;
        if (--it_a != mapAsm.end()) {
          DrawString(x, nLineY, (*it_a).second);
        }
      }
    }
  }

  bool OnUserCreate() {
    // Load Program (assembled at https://www.masswerk.at/6502/assembler.html)
    /*
            *=$8000
            LDX #10
            STX $0000
            LDX #3
            STX $0001
            LDY $0000
            LDA #0
            CLC
            loop
            ADC $0001
            DEY
            BNE loop
            STA $0002
            NOP
            NOP
            NOP
    */

    // Convert hex string into bytes for RAM
    std::stringstream ss;
    ss << "A2 0A 8E 00 00 A2 03 8E 01 00 AC 00 00 A9 00 18 6D 01 00 88 D0 FA "
          "8D 02 00 EA EA EA";
    uint16_t nOffset = 0x8000;
    while (!ss.eof()) {
      std::string b;
      ss >> b;
      nes.ram[nOffset++] = (uint8_t)std::stoul(b, nullptr, 16);
    }

    // Set Reset Vector
    nes.ram[0xFFFC] = 0x00;
    nes.ram[0xFFFD] = 0x80;

    // Dont forget to set IRQ and NMI vectors if you want to play with those

    // Extract dissassembly
    mapAsm = nes.cpu.disassemble(0x0000, 0xFFFF);

    // Reset
    nes.cpu.reset();
    return true;
  }

  bool OnUserUpdate(float fElapsedTime) {
    Clear(olc::DARK_BLUE);

    if (GetKey(olc::Key::SPACE).bPressed) {
      do {
        nes.cpu.clock();
      } while (!nes.cpu.complete());
    }

    if (GetKey(olc::Key::R).bPressed)
      nes.cpu.reset();

    if (GetKey(olc::Key::I).bPressed)
      nes.cpu.irq();

    if (GetKey(olc::Key::N).bPressed)
      nes.cpu.nmi();

    // Draw Ram Page 0x00
    DrawRam(2, 2, 0x0000, 16, 16);
    DrawRam(2, 182, 0x8000, 16, 16);
    DrawCpu(448, 2);
    DrawCode(448, 72, 26);

    DrawString(10, 370,
               "SPACE = Step Instruction    R = RESET    I = IRQ    N = NMI");

    return true;
  }
};

int main(int argc, char const *argv[]) {
  DemoM6502 demo;
  demo.Construct(680, 480, 2, 2);
  demo.Start();
  return 0;
}
