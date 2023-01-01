#pragma once

#include "Cartridge.h"
#include "IMapper.h"

class MapperFactory
{
  public:
    static IMapper *createMapper(Cartridge &cartridge);
};