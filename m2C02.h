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

  uint8_t cpuRead(uint16_t addr, bool bReadOnly = false) const;
  void cpuWrite(uint16_t addr, uint8_t data);
  uint8_t ppuRead(uint16_t addr, bool bReadOnly = false) const;
  void ppuWrite(uint16_t addr, uint8_t data);

  void ConnectCartridge(const std::shared_ptr<Catridge> &catridge)
  {
    cart = catridge;
  }
  void clock();

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
  bool nmi{false};

  std::array<std::array<uint8_t, 1024>, 2> tblName;

private:
  std::shared_ptr<Catridge> cart;
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

  // Loopy Register, refer to https://wiki.nesdev.com/w/index.php/PPU_scrolling
  union loopyRegister
  {
    struct
    {
      uint16_t coarse_x : 5;
      uint16_t coarse_y : 5;
      uint16_t nametable_x : 1;
      uint16_t nametable_y : 1;
      uint16_t fine_y : 3;
      uint16_t unused : 1;
    };
    uint16_t reg = 0x0000u;
  };

  mutable uint8_t addressLatch{0x00u};
  mutable uint8_t ppuDataBuffer{0x00u};

  mutable loopyRegister vramAddr;
  mutable loopyRegister tramAddr;

  uint8_t fineX{0x00u};

  uint8_t bgNextTileId{0x00u};
  uint8_t bgNextTileAttrib{0x00u};
  uint8_t bgNextTileLsb{0x00u};
  uint8_t bgNextTileMsb{0x00u};

  uint16_t bgShifterPatternLo{0x0000u};
  uint16_t bgShifterPatternHi{0x0000u};
  uint16_t bgShifterAttribLo{0x0000u};
  uint16_t bgShifterAttribHi{0x0000u};
};