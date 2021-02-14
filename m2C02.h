#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include "Catridge.h"

#include "olcPixelGameEngine.h"

class m2C02
{
public:
  m2C02();
  ~m2C02() {}

  uint8_t cpuRead(uint16_t addr, bool bReadOnly = false) const
  {
    uint8_t data = 0x00u;
    if (bReadOnly)
    {
      switch (addr)
      {
      case 0x0000u: // Control
        data = control.reg;
        break;
      case 0x0001u: // Mask
        data = mask.reg;
        break;
      case 0x0002u: // Status
        data = status.reg;
        break;
      case 0x0003u: // OAM Address
        break;
      case 0x0004u: // OAM Data
        break;
      case 0x0005u: // Scroll
        break;
      case 0x0006u: // PPU Address
        break;
      case 0x0007u: // PPU Data
        break;
      default:
        break;
      }
    }
    else
    {
      switch (addr)
      {
      case 0x0000u: // Control
        break;
      case 0x0001u: // Mask
        break;
      case 0x0002u: // Status
        status.vertical_blank = 1;
        data = (status.reg & 0xE0u) | (ppuDataBuffer & 0x1Fu);
        status.vertical_blank = 0;
        addressLatch = 0;
        break;
      case 0x0003u: // OAM Address
        break;
      case 0x0004u: // OAM Data
        break;
      case 0x0005u: // Scroll
        break;
      case 0x0006u: // PPU Address
        break;
      case 0x0007u: // PPU Data
        data = ppuDataBuffer;
        ppuDataBuffer = ppuRead(ppuAddress);

        if (ppuAddress++ > 0x3F00u)
        {
          data = ppuDataBuffer;
        }
        break;
      default:
        break;
      }
    }

    return data;
  }
  void cpuWrite(uint16_t addr, uint8_t data)
  {
    switch (addr)
    {
    case 0x0000u: // Control
      control.reg = data;
      break;
    case 0x0001u: // Mask
      mask.reg = data;
      break;
    case 0x0002u: // Status
      break;
    case 0x0003u: // OAM Address
      break;
    case 0x0004u: // OAM Data
      break;
    case 0x0005u: // Scroll
      break;
    case 0x0006u: // PPU Address
      if (addressLatch == 0)
      {
        ppuAddress = (ppuAddress & 0x00FFu) | (data << 8);
        addressLatch = 1;
      }
      else
      {
        ppuAddress = (ppuAddress & 0xFF00u) | data;
        addressLatch = 0;
      }
      break;
    case 0x0007u: // PPU Data
      ppuWrite(ppuAddress++, data);
      break;
    default:
      break;
    }
  }

  uint8_t ppuRead(uint16_t addr, bool bReadOnly = false) const
  {
    uint8_t data = 0x00u;
    addr &= 0x3FFFu;

    if (cart->ppuRead(addr, data))
    {
    }
    else if (addr >= 0x0000u && addr <= 0x1FFFu)
    {
      data = tblPattern[(addr & 0x1000u) >> 12][addr & 0x0FFFu];
    }
    else if (addr >= 0x2000u && addr <= 0x3EFFu)
    {
    }
    else if (addr >= 0x3F00u && addr <= 0x3FFFu)
    {
      addr &= 0x001Fu;
      switch (addr)
      {
      case 0x0010u:
        addr = 0x0000u;
        break;
      case 0x0014u:
        addr = 0x0004u;
        break;
      case 0x0018u:
        addr = 0x0008u;
        break;
      case 0x001Cu:
        addr = 0x000Cu;
        break;
      default:
        break;
      }
      data = tblPalette[addr];
    }

    return data;
  }
  void ppuWrite(uint16_t addr, uint8_t data)
  {
    addr &= 0x3FFFu;
    if (cart->ppuWrite(addr, data))
    {
    }
    else if (addr >= 0x0000u && addr <= 0x1FFFu)
    {
      tblPattern[(addr & 0x1000u) >> 12][addr & 0x0FFFu] = data;
    }
    else if (addr >= 0x2000u && addr <= 0x3EFFu)
    {
    }
    else if (addr >= 0x3F00u && addr <= 0x3FFFu)
    {
      addr &= 0x001Fu;
      switch (addr)
      {
      case 0x0010u:
        addr = 0x0000u;
        break;
      case 0x0014u:
        addr = 0x0004u;
        break;
      case 0x0018u:
        addr = 0x0008u;
        break;
      case 0x001Cu:
        addr = 0x000Cu;
        break;
      default:
        break;
      }
      tblPalette[addr] = data;
    }
  }

  void ConnectCartridge(const std::shared_ptr<Catridge> &catridge)
  {
    cart = catridge;
  }
  void clock()
  {
    // Fake some noise
    sprScreen.SetPixel(cycle - 1, scanline,
                       palScreen[(rand() % 2) ? 0x3F : 0x30]);

    ++cycle;
    if (cycle >= 341)
    {
      cycle = 0;
      ++scanline;
      if (scanline >= 261)
      {
        scanline = -1;
        frameComplete = true;
      }
    }
  }

  // Debugging Utilities
  olc::Sprite &GetScreen() { return sprScreen; }
  olc::Sprite &GetNameTabel(uint8_t i) { return sprNameTable[i]; }
  olc::Sprite &GetPatternTable(uint8_t i, uint8_t palette)
  {
    for (uint16_t nTileY = 0; nTileY < 16; nTileY++)
    {
      for (uint16_t nTileX = 0; nTileX < 16; nTileX++)
      {
        uint16_t nOffset = nTileY * 256 + nTileX * 16;

        for (uint16_t row = 0; row < 8; row++)
        {
          uint8_t tileLsb = ppuRead(i * 0x1000u + nOffset + row + 0);
          uint8_t tileMsb = ppuRead(i * 0x1000u + nOffset + row + 8);

          for (uint16_t col = 0; col < 8; col++)
          {
            uint8_t pixel = (tileLsb & 0x01u) + (tileMsb & 0x01u);
            tileLsb >>= 1;
            tileMsb >>= 1;
            sprPatternTable[i].SetPixel(
                nTileX * 8 + (7 - col), nTileY * 8 + row,
                GetColourFromPaletteRam(palette, pixel));
          }
        }
      }
    }

    return sprPatternTable[i];
  }
  olc::Pixel &GetColourFromPaletteRam(uint8_t palette, uint8_t pixel)
  {
    return palScreen[ppuRead(0x3F00 + (palette << 2) + pixel) & 0x3F];
  }

  bool frameComplete{false};

private:
  std::shared_ptr<Catridge> cart;
  std::array<std::array<uint8_t, 1024>, 2> tblName;
  std::array<uint8_t, 32> tblPalette;
  // TODO: Javid Future
  std::array<std::array<uint8_t, 4096>, 2> tblPattern;

  std::array<olc::Pixel, 0x40> palScreen;
  olc::Sprite sprScreen{olc::Sprite(256, 240)};
  std::array<olc::Sprite, 2> sprNameTable{
      {olc::Sprite(256, 240), olc::Sprite(256, 240)}};
  std::array<olc::Sprite, 2> sprPatternTable{
      {olc::Sprite(128, 128), olc::Sprite(128, 128)}};

  int16_t scanline{0};
  int16_t cycle{0};

  union
  {
    struct
    {
      uint8_t unused : 5;
      uint8_t sprite_overflow : 1;
      uint8_t sprite_zero_hit : 1;
      mutable uint8_t vertical_blank : 1;
    };
    uint8_t reg;
  } status;

  union
  {
    struct
    {
      uint8_t grayscale : 1;
      uint8_t render_background_left : 1;
      uint8_t render_sprites_left : 1;
      uint8_t render_background : 1;
      uint8_t render_sprites : 1;
      uint8_t enhance_red : 1;
      uint8_t enhance_green : 1;
      uint8_t enhance_blue : 1;
    };
    uint8_t reg;
  } mask;

  union PPUCTRL
  {
    struct
    {
      uint8_t nametable_x : 1;
      uint8_t nametable_y : 1;
      uint8_t increment_mode : 1;
      uint8_t pattern_sprite : 1;
      uint8_t pattern_background : 1;
      uint8_t sprite_size : 1;
      uint8_t slave_mode : 1; // unused
      uint8_t enable_nmi : 1;
    };
    uint8_t reg;
  } control;

  mutable uint8_t addressLatch{0x00u};
  mutable uint8_t ppuDataBuffer{0x00u};
  mutable uint16_t ppuAddress{0x0000u};
};