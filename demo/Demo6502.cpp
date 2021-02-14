#include "Bus.h"
#include "Catridge.h"
#include "m6502.h"

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

using namespace std;

class DemoM6502 : public olc::PixelGameEngine
{
public:
  DemoM6502() { sAppName = "m6502 Demonstration"; }

  shared_ptr<Catridge> cart;
  Bus nes;
  map<uint16_t, string> mapAsm;

  bool bEmulationRun{false};
  float fResidualTime{0.0f};

  uint8_t nSelectedPalette{0x00u};

  string hex(uint32_t n, uint8_t d)
  {
    string s(d, '0');
    for (int i = d - 1; i >= 0; i--, n >>= 4)
      s[i] = "0123456789ABCDEF"[n & 0xF];
    return s;
  };

  void DrawRam(int x, int y, uint16_t nAddr, int nRows, int nColumns)
  {
    int nRamX = x, nRamY = y;
    for (int row = 0; row < nRows; row++)
    {
      string sOffset = "$" + hex(nAddr, 4) + ":";
      for (int col = 0; col < nColumns; col++)
      {
        sOffset += " " + hex(nes.cpuRead(nAddr, true), 2);
        nAddr += 1;
      }
      DrawString(nRamX, nRamY, sOffset);
      nRamY += 10;
    }
  }

  void DrawCpu(int x, int y)
  {
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

  void DrawCode(int x, int y, int nLines)
  {
    auto it_a = mapAsm.find(nes.cpu.pc);
    int nLineY = (nLines >> 1) * 10 + y;
    if (it_a != mapAsm.end())
    {
      DrawString(x, nLineY, (*it_a).second, olc::CYAN);
      while (nLineY < (nLines * 10) + y)
      {
        nLineY += 10;
        if (++it_a != mapAsm.end())
        {
          DrawString(x, nLineY, (*it_a).second);
        }
      }
    }

    it_a = mapAsm.find(nes.cpu.pc);
    nLineY = (nLines >> 1) * 10 + y;
    if (it_a != mapAsm.end())
    {
      while (nLineY > y)
      {
        nLineY -= 10;
        if (--it_a != mapAsm.end())
        {
          DrawString(x, nLineY, (*it_a).second);
        }
      }
    }
  }

  bool OnUserCreate()
  {
    cart = make_shared<Catridge>("nestest.nes");
    if (!cart->ImageValid())
    {
      return false;
    }
    nes.insertCartridge(cart);
    mapAsm = nes.cpu.disassemble(0x0000u, 0xFFFFu);
    nes.reset();
    return true;
  }

  bool OnUserUpdate(float fElapsedTime)
  {
    Clear(olc::DARK_BLUE);

    if (GetKey(olc::Key::R).bPressed)
    {
      nes.cpu.reset();
    }
    if (GetKey(olc::Key::SPACE).bPressed)
    {
      bEmulationRun = !bEmulationRun;
    }
    if (GetKey(olc::Key::P).bPressed)
    {
      (++nSelectedPalette) &= 0x07u;
    }

    if (bEmulationRun)
    {
      if (fResidualTime > 0.0f)
      {
        fResidualTime -= fElapsedTime;
      }
      else
      {
        // 60 Hz
        fResidualTime += (1.0f / 60.0f) - fElapsedTime;
        do
        {
          nes.clock();
        } while (!nes.ppu.frameComplete);
        nes.ppu.frameComplete = false;
      }
    }
    else
    {
      // emulate code step-by-step
      if (GetKey(olc::Key::C).bPressed)
      {
        do
        {
          nes.cpu.clock();
        } while (!nes.cpu.complete());

        // Drain remain CPU clocks
        do
        {
          nes.cpu.clock();
        } while (nes.cpu.complete());
      }

      // emulate one whole frame
      if (GetKey(olc::Key::F).bPressed)
      {
        do
        {
          nes.clock();
        } while (!nes.ppu.frameComplete);

        // Use residual clock cycles to complete current instruction
        do
        {
          nes.clock();
        } while (!nes.cpu.complete());
        nes.ppu.frameComplete = false;
      }
    }

    // Draw Ram Page 0x00
    DrawCpu(516, 2);
    DrawCode(516, 72, 26);

    constexpr int nSwatchSize = 6;
    for (int p = 0; p < 8; p++)
    {
      for (int s = 0; s < 4; s++)
      {
        FillRect(516 + p * (nSwatchSize * 5) + s * nSwatchSize, 340,
                 nSwatchSize, nSwatchSize,
                 nes.ppu.GetColourFromPaletteRam(p, s));
      }
    }

    DrawRect(516 + nSelectedPalette * (nSwatchSize * 5) - 1, 339,
             (nSwatchSize * 4), nSwatchSize, olc::WHITE);

    DrawSprite(516, 348, &nes.ppu.GetPatternTable(0, nSelectedPalette));
    DrawSprite(648, 348, &nes.ppu.GetPatternTable(1, nSelectedPalette));

    DrawSprite(0, 0, &nes.ppu.GetScreen(), 2);

    return true;
  }
};

int main(int argc, char const *argv[])
{
  DemoM6502 demo;
  demo.Construct(780, 480, 2, 2);
  demo.Start();
  return 0;
}
