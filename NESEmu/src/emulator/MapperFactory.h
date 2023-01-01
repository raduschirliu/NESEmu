#pragma once

#include "Cartridge.h"
#include "IMapper.h"

class MapperFactory
{
  public:
    static IMapper *CreateMapper(Cartridge &cartridge);
};