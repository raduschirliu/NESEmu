#include "MapperFactory.h"

#include "../mappers/NROM.h"

IMapper *MapperFactory::createMapper(Cartridge &cartridge)
{
	switch (cartridge.getMapperID())
	{
	case 0:
		return new mappers::NROM(cartridge);
	}

	return nullptr;
}
