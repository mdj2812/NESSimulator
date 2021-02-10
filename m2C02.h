#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include "Catridge.h"

#include "olcPixelGameEngine.h"

class m2C02 {
public:
  m2C02();
  ~m2C02() {}

  uint8_t cpuRead(uint16_t addr, bool bReadOnly = false) const {
    uint8_t data = 0x00u;
    switch (addr) {
    case 0x0000u: // Control
      break;
    case 0x0001u: // Mask
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
      break;
    case 0x0007u: // PPU Data
      break;
    default:
      break;
    }
    return data;
  }
  void cpuWrite(uint16_t addr, uint8_t data) {
    switch (addr) {
    case 0x0000u: // Control
      break;
    case 0x0001u: // Mask
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
      break;
    case 0x0007u: // PPU Data
      break;
    default:
      break;
    }
  }

  uint8_t ppuRead(uint16_t addr, bool bReadOnly = false) const {
    uint8_t data = 0x00u;
    addr &= 0x3FFFu;

    if (cart->ppuRead(addr, data)) {
    }
    return data;
  }
  void ppuWrite(uint16_t addr, uint8_t data) {
    addr &= 0x3FFFu;
    if (cart->ppuWrite(addr, data)) {
    }
  }

  void ConnectCartridge(const std::shared_ptr<Catridge> &catridge) {
    cart = catridge;
  }
  void clock() {
    // Fake some noise
    sprScreen.SetPixel(cycle - 1, scanline,
                       palScreen[(rand() % 2) ? 0x3F : 0x30]);

    ++cycle;
    if (cycle >= 341) {
      cycle = 0;
      ++scanline;
      if (scanline >= 261) {
        scanline = -1;
        frameComplete = true;
      }
    }
  }

  // Debugging Utilities
  olc::Sprite &GetScreen() { return sprScreen; }
  olc::Sprite &GetNameTabel(uint8_t i) { return sprNameTable[i]; }
  olc::Sprite &GetPatternTable(uint8_t i) { return sprPatternTable[i]; }
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
};