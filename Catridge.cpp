#include "Catridge.h"

#include <fstream>

using namespace std;

Catridge::Catridge(const string &sFileName)
{
  struct sHeader
  {
    char name[4];
    uint8_t prg_rom_chunks;
    uint8_t chr_rom_chunks;
    uint8_t mapper1;
    uint8_t mapper2;
    uint8_t prg_ram_size;
    uint8_t tv_system1;
    uint8_t tv_system2;
    char unused[5];
  } header;

  std::ifstream ifs(sFileName, ifstream::binary);
  if (ifs.is_open())
  {
    ifs.read((char *)&header, sizeof(sHeader));

    if (header.mapper1 & 0x04u)
    {
      ifs.seekg(512, ios_base::cur);
    }

    nMapperID = ((header.mapper2 >> 4) << 4) | (header.mapper1 >> 4);
    mirror = (header.mapper1 & 0x01u) ? VERTICAL : HORIZONTAL;

    // "Discover" File format
    uint8_t nFileType = 1;

    if (nFileType == 0)
    {
    }

    if (nFileType == 1)
    {
      nPRGBanks = header.prg_rom_chunks;
      vPRGMemory.resize(nPRGBanks * 16384); // nPRGBanks * 16 KB
      ifs.read((char *)vPRGMemory.data(), vPRGMemory.size());

      nCHRBanks = header.chr_rom_chunks;
      if (nCHRBanks == 0)
      {
        vCHRMemory.resize(8192); // 8 KB
      }
      else
      {
        vCHRMemory.resize(nCHRBanks * 8192); // nPRGBanks * 8 KB
      }
      ifs.read((char *)vCHRMemory.data(), vCHRMemory.size());
    }

    if (nFileType == 2)
    {
    }

    switch (nMapperID)
    {
    case 0:
      pMapper = make_shared<Mapper_000>(nPRGBanks, nCHRBanks);
      break;
    default:
      break;
    }

    bImageValid = true;
  }
}
