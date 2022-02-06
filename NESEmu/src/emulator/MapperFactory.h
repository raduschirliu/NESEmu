#pragma once

#include "IMapper.h"
#include "Cartridge.h"

class MapperFactory
{
public:
	static IMapper *createMapper(Cartridge &cartridge);
};