#include "MapperFactory.h"

#include "emulator/mappers/NROM.h"

IMapper *MapperFactory::CreateMapper(Cartridge &cartridge)
{
    switch (cartridge.GetMapperId())
    {
        case 0:
            return new mappers::NROM(cartridge);
    }

    // TODO(radu): Throw error for unimplemented mapper?
    return nullptr;
}
