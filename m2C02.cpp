#include "m2C02.h"

m2C02::m2C02()
{
  // refer to https://wiki.nesdev.com/w/index.php/PPU_palettes
  palScreen[0x00] = olc::Pixel(84, 84, 84);
  palScreen[0x01] = olc::Pixel(0, 30, 116);
  palScreen[0x02] = olc::Pixel(8, 16, 144);
  palScreen[0x03] = olc::Pixel(48, 0, 136);
  palScreen[0x04] = olc::Pixel(68, 0, 100);
  palScreen[0x05] = olc::Pixel(92, 0, 48);
  palScreen[0x06] = olc::Pixel(84, 4, 0);
  palScreen[0x07] = olc::Pixel(60, 24, 0);
  palScreen[0x08] = olc::Pixel(32, 42, 0);
  palScreen[0x09] = olc::Pixel(8, 58, 0);
  palScreen[0x0A] = olc::Pixel(0, 64, 0);
  palScreen[0x0B] = olc::Pixel(0, 60, 0);
  palScreen[0x0C] = olc::Pixel(0, 50, 60);
  palScreen[0x0D] = olc::Pixel(0, 0, 0);
  palScreen[0x0E] = olc::Pixel(0, 0, 0);
  palScreen[0x0F] = olc::Pixel(0, 0, 0);

  palScreen[0x10] = olc::Pixel(152, 150, 152);
  palScreen[0x11] = olc::Pixel(8, 76, 196);
  palScreen[0x12] = olc::Pixel(48, 50, 236);
  palScreen[0x13] = olc::Pixel(92, 30, 228);
  palScreen[0x14] = olc::Pixel(136, 20, 176);
  palScreen[0x15] = olc::Pixel(160, 20, 100);
  palScreen[0x16] = olc::Pixel(152, 34, 32);
  palScreen[0x17] = olc::Pixel(120, 60, 0);
  palScreen[0x18] = olc::Pixel(84, 90, 0);
  palScreen[0x19] = olc::Pixel(40, 114, 0);
  palScreen[0x1A] = olc::Pixel(8, 124, 0);
  palScreen[0x1B] = olc::Pixel(0, 118, 40);
  palScreen[0x1C] = olc::Pixel(0, 102, 120);
  palScreen[0x1D] = olc::Pixel(0, 0, 0);
  palScreen[0x1E] = olc::Pixel(0, 0, 0);
  palScreen[0x1F] = olc::Pixel(0, 0, 0);

  palScreen[0x20] = olc::Pixel(236, 238, 236);
  palScreen[0x21] = olc::Pixel(76, 154, 236);
  palScreen[0x22] = olc::Pixel(120, 124, 236);
  palScreen[0x23] = olc::Pixel(176, 98, 236);
  palScreen[0x24] = olc::Pixel(228, 84, 236);
  palScreen[0x25] = olc::Pixel(236, 88, 180);
  palScreen[0x26] = olc::Pixel(236, 106, 100);
  palScreen[0x27] = olc::Pixel(212, 136, 32);
  palScreen[0x28] = olc::Pixel(160, 170, 0);
  palScreen[0x29] = olc::Pixel(116, 196, 0);
  palScreen[0x2A] = olc::Pixel(76, 208, 32);
  palScreen[0x2B] = olc::Pixel(56, 204, 108);
  palScreen[0x2C] = olc::Pixel(56, 180, 204);
  palScreen[0x2D] = olc::Pixel(60, 60, 60);
  palScreen[0x2E] = olc::Pixel(0, 0, 0);
  palScreen[0x2F] = olc::Pixel(0, 0, 0);

  palScreen[0x30] = olc::Pixel(236, 238, 236);
  palScreen[0x31] = olc::Pixel(168, 204, 236);
  palScreen[0x32] = olc::Pixel(188, 188, 236);
  palScreen[0x33] = olc::Pixel(212, 178, 236);
  palScreen[0x34] = olc::Pixel(236, 174, 236);
  palScreen[0x35] = olc::Pixel(236, 174, 212);
  palScreen[0x36] = olc::Pixel(236, 180, 176);
  palScreen[0x37] = olc::Pixel(228, 196, 144);
  palScreen[0x38] = olc::Pixel(204, 210, 120);
  palScreen[0x39] = olc::Pixel(180, 222, 120);
  palScreen[0x3A] = olc::Pixel(168, 226, 144);
  palScreen[0x3B] = olc::Pixel(152, 226, 180);
  palScreen[0x3C] = olc::Pixel(160, 214, 228);
  palScreen[0x3D] = olc::Pixel(160, 162, 160);
  palScreen[0x3E] = olc::Pixel(0, 0, 0);
  palScreen[0x3F] = olc::Pixel(0, 0, 0);
}

uint8_t m2C02::cpuRead(uint16_t addr, bool bReadOnly) const
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
      ppuDataBuffer = ppuRead(vramAddr.reg);

      if (vramAddr.reg >= 0x3F00u)
      {
        data = ppuDataBuffer;
      }
      vramAddr.reg += (control.increment_mode ? 32 : 1);
      break;
    default:
      break;
    }
  }

  return data;
}
void m2C02::cpuWrite(uint16_t addr, uint8_t data)
{
  switch (addr)
  {
  case 0x0000u: // Control
    control.reg = data;
    tramAddr.nametable_x = control.nametable_x;
    tramAddr.nametable_y = control.nametable_y;
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
    if (addressLatch == 0)
    {
      fineX = data & 0x07u;
      tramAddr.coarse_x = data >> 3;
      addressLatch = 1;
    }
    else
    {
      tramAddr.fine_y = data & 0x07u;
      tramAddr.coarse_y = data >> 3;
      addressLatch = 0;
    }
    break;
  case 0x0006u: // PPU Address
    if (addressLatch == 0)
    {
      tramAddr.reg = (tramAddr.reg & 0x00FFu) | (data << 8);
      addressLatch = 1;
    }
    else
    {
      tramAddr.reg = (tramAddr.reg & 0xFF00u) | data;
      vramAddr = tramAddr;
      addressLatch = 0;
    }
    break;
  case 0x0007u: // PPU Data
    ppuWrite(vramAddr.reg, data);
    vramAddr.reg += (control.increment_mode ? 32 : 1);
    break;
  default:
    break;
  }
}

uint8_t m2C02::ppuRead(uint16_t addr, bool bReadOnly) const
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
    addr &= 0x0FFF;
    if (cart->mirror == Catridge::MIRROR::VERTICAL)
    {
      if (addr >= 0x0000u && addr <= 0x03FFu)
      {
        data = tblName[0][addr & 0x03FFu];
      }
      else if (addr >= 0x0400u && addr <= 0x07FFu)
      {
        data = tblName[1][addr & 0x03FFu];
      }
      else if (addr >= 0x0800u && addr <= 0x0BFFu)
      {
        data = tblName[0][addr & 0x03FFu];
      }
      else if (addr >= 0x0C00u && addr <= 0x0FFFu)
      {
        data = tblName[1][addr & 0x03FFu];
      }
    }
    else if (cart->mirror == Catridge::MIRROR::HORIZONTAL)
    {
      if (addr >= 0x0000u && addr <= 0x03FFu)
      {
        data = tblName[0][addr & 0x03FFu];
      }
      else if (addr >= 0x0400u && addr <= 0x07FFu)
      {
        data = tblName[0][addr & 0x03FFu];
      }
      else if (addr >= 0x0800u && addr <= 0x0BFFu)
      {
        data = tblName[1][addr & 0x03FFu];
      }
      else if (addr >= 0x0C00u && addr <= 0x0FFFu)
      {
        data = tblName[1][addr & 0x03FFu];
      }
    }
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
void m2C02::ppuWrite(uint16_t addr, uint8_t data)
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
    addr &= 0x0FFFu;
    if (cart->mirror == Catridge::MIRROR::VERTICAL)
    {
      if (addr >= 0x0000u && addr <= 0x03FFu)
      {
        tblName[0][addr & 0x03FFu] = data;
      }
      else if (addr >= 0x0400u && addr <= 0x07FFu)
      {
        tblName[1][addr & 0x03FFu] = data;
      }
      else if (addr >= 0x0800u && addr <= 0x0BFFu)
      {
        tblName[0][addr & 0x03FFu] = data;
      }
      else if (addr >= 0x0C00u && addr <= 0x0FFFu)
      {
        tblName[1][addr & 0x03FFu] = data;
      }
    }
    else if (cart->mirror == Catridge::MIRROR::HORIZONTAL)
    {
      if (addr >= 0x0000u && addr <= 0x03FFu)
      {
        tblName[0][addr & 0x03FFu] = data;
      }
      else if (addr >= 0x0400u && addr <= 0x07FFu)
      {
        tblName[0][addr & 0x03FFu] = data;
      }
      else if (addr >= 0x0800u && addr <= 0x0BFFu)
      {
        tblName[1][addr & 0x03FFu] = data;
      }
      else if (addr >= 0x0C00u && addr <= 0x0FFFu)
      {
        tblName[1][addr & 0x03FFu] = data;
      }
    }
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

void m2C02::clock()
{
  auto IncrementScrollX = [&]() {
    if (mask.render_background || mask.render_sprites)
    {
      if (vramAddr.coarse_x == 31)
      {
        vramAddr.coarse_x = 0;
        vramAddr.nametable_x = ~vramAddr.nametable_x;
      }
      else
      {
        ++vramAddr.coarse_x;
      }
    }
  };
  auto IncrementScrollY = [&]() {
    if (mask.render_background || mask.render_sprites)
    {
      if (vramAddr.fine_y < 7)
      {
        ++vramAddr.fine_y;
      }
      else
      {
        vramAddr.fine_y = 0;
        if (vramAddr.coarse_y == 29)
        {
          vramAddr.coarse_y = 0;
          vramAddr.nametable_y = ~vramAddr.nametable_y;
        }
        else if (vramAddr.nametable_y == 31)
        {
          vramAddr.coarse_y = 0;
        }
        else
        {
          ++vramAddr.coarse_y;
        }
      }
    }
  };
  auto TransferAddressX = [&]() {
    if (mask.render_background || mask.render_sprites)
    {
      vramAddr.nametable_x = tramAddr.nametable_x;
      vramAddr.coarse_x = tramAddr.coarse_x;
    }
  };
  auto TransferAddressY = [&]() {
    if (mask.render_background || mask.render_sprites)
    {
      vramAddr.fine_y = tramAddr.fine_y;
      vramAddr.nametable_y = tramAddr.nametable_y;
      vramAddr.coarse_y = tramAddr.coarse_y;
    }
  };
  auto LoadBackgroundShifters = [&]() {
    bgShifterPatternLo = (bgShifterPatternLo & 0xFF00u) | bgNextTileLsb;
    bgShifterPatternHi = (bgShifterPatternHi & 0xFF00u) | bgNextTileMsb;
    bgShifterAttribLo = (bgShifterAttribLo & 0xFF00u) | ((bgNextTileAttrib & 0b01) ? 0xFFu : 0x00u);
    bgShifterAttribHi = (bgShifterAttribHi & 0xFF00u) | ((bgNextTileAttrib & 0b10) ? 0xFFu : 0x00u);
  };
  auto UpdateShifters = [&]() {
    if (mask.render_background)
    {
      bgShifterPatternLo <<= 1;
      bgShifterPatternHi <<= 1;
      bgShifterAttribLo <<= 1;
      bgShifterAttribHi <<= 1;
    }
  };

  if (scanline >= -1 && scanline < 240)
  {
    if (scanline == -1 && cycle == 1)
    {
      status.vertical_blank = 0;
    }

    if ((cycle >= 2 && cycle < 258) || (cycle >= 321 && cycle < 338))
    {
      UpdateShifters();

      switch ((cycle - 1) % 8)
      {
      case 0:
        LoadBackgroundShifters();
        bgNextTileId = ppuRead(0x2000u | (vramAddr.reg & 0x0FFF));
        break;
      case 2:
        bgNextTileAttrib = ppuRead(0x23C0 | (vramAddr.nametable_y << 11) | (vramAddr.nametable_x << 10) | ((vramAddr.coarse_y >> 2) << 3) | (vramAddr.coarse_x >> 2));
        if (vramAddr.coarse_y & 0x02u)
        {
          bgNextTileAttrib >>= 4;
        }
        if (vramAddr.coarse_x & 0x02u)
        {
          bgNextTileAttrib >>= 2;
        }
        bgNextTileAttrib &= 0x03u;
        break;
      case 4:
        bgNextTileLsb = ppuRead((control.pattern_background << 12) + ((uint16_t)bgNextTileId << 4) + (vramAddr.fine_y) + 0);
        break;
      case 6:
        bgNextTileMsb = ppuRead((control.pattern_background << 12) + ((uint16_t)bgNextTileId << 4) + (vramAddr.fine_y) + 8);
        break;
      case 7:
        IncrementScrollX();
        break;
      default:
        break;
      }
    }

    if (cycle == 256)
    {
      IncrementScrollY();
    }

    if (cycle == 257)
    {
      TransferAddressX();
    }

    if (scanline == -1 && cycle >= 280 && cycle < 305)
    {
      TransferAddressY();
    }
  }

  if (scanline == 240)
  {
    // Do nothing
  }

  if (scanline == 241 && cycle == 1)
  {
    status.vertical_blank = 1;
    if (control.enable_nmi)
    {
      nmi = true;
    }
  }

  uint8_t bgPixel = 0x00u;
  uint8_t bgPalette = 0x00u;

  if (mask.render_background)
  {
    uint16_t bitMux = 0x8000u >> fineX;

    uint8_t p0Pixel = (bgShifterPatternLo & bitMux) > 0;
    uint8_t p1Pixel = (bgShifterPatternHi & bitMux) > 0;
    bgPixel = (p1Pixel << 1) | p0Pixel;

    uint8_t bgPal0 = (bgShifterAttribLo & bitMux) > 0;
    uint8_t bgPal1 = (bgShifterAttribHi & bitMux) > 0;
    bgPalette = (bgPal1 << 1) | bgPal0;
  }

  sprScreen.SetPixel(cycle - 1, scanline,
                     GetColourFromPaletteRam(bgPalette, bgPixel));

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
