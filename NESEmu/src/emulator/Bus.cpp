#include "Bus.h"

#include <iomanip>
#include <sstream>

// PPU register locations
static uint16_t constexpr PPU_OAMDATA = 0x4014;

Bus::Bus()
{
    // Allocate memory arrays for all NES components
    cpuMem = new uint8_t[2048]();
    ppuRegisterMem = new uint8_t[8]();
    apuMem = new uint8_t[24]();
    testMem = new uint8_t[8]();
    mapper = nullptr;

    // Other things needed on the bus
    shouldDispatchNmi = false;
    shouldDispatchOamTransfer = false;
}

Bus::~Bus()
{
    // Deallocate all memory arrays
    delete[] cpuMem;
    delete[] ppuRegisterMem;
    delete[] apuMem;
    delete[] testMem;
}

uint8_t *Bus::get(uint16_t address)
{
    if (address >= 0x0000 && address <= 0x1FFF)
    {
        // Get from CPU memory ($0000 - $1FFF, mirrored > $07FF)
        uint16_t target = address % 0x0800;
        return &cpuMem[target];
    }
    else if (address >= 0x2000 && address <= 0x3FFF)
    {
        // Get from PPU register memory ($2000 - $3FFF, mirrored > $2007)
        uint16_t target = (address - 0x2000) % 0x0008;
        return &ppuRegisterMem[target];
    }
    else if (address >= 0x4000 && address <= 0x4017)
    {
        // Get from APU & I/O Memory ($4000 - $4017)
        return &apuMem[address - 0x4000];
    }
    else if (address >= 0x4018 && address <= 0x401F)
    {
        // Get from APU & I/O test Memory (usually disabled) ($4018 - $401F)
        return &testMem[address - 0x4018];
    }

    return nullptr;
}

uint8_t Bus::read(uint16_t address, bool skipCallback)
{
    // Get from cartridge Memory ($4020 - $FFFF)
    if (address >= 0x4020 && address <= 0xFFFF)
    {
        return mapper->PrgRead(address);
    }

    // Any other bus accesible memory (< $4020)
    uint8_t *ptr = get(address);

    if (ptr != nullptr)
    {
        uint8_t val = *ptr;

        if (!skipCallback)
        {
            dispatchMemoryAccessCallbacks(address, 0, false);
        }

        return val;
    }

    // TODO: Return an error value?
    return 0;
}

void Bus::write(uint16_t address, uint8_t value, bool skipCallback)
{
    // Get from cartridge Memory ($4020 - $FFFF)
    if (address >= 0x4020 && address <= 0xFFFF)
    {
        mapper->PrgWrite(address, value);
        return;
    }

    // Any other bus accesible memory (< $4020)
    uint8_t *ptr = get(address);

    if (ptr != nullptr)
    {
        *ptr = value;

        if (!skipCallback)
        {
            dispatchMemoryAccessCallbacks(address, value, true);
        }
    }
}

void Bus::dump(Logger &logger)
{
    std::stringstream ss;

    ss << "Full memory dump\n"
       << "--------------------\n"
       << std::hex;

    // Dump 8 bytes per line
    for (int base = 0; base <= 0xFFFF; base += 8)
    {
        ss << std::setw(4) << std::setfill('0') << base << ":\t";

        for (int line = 0; line < 8; line++)
        {
            ss << std::setw(2) << std::setfill('0') << (int)read(base + line)
               << " ";
        }

        ss << std::endl;
    }

    logger.Write(ss.str());
}

void Bus::registerMemoryAccessCallback(AccessCallback callback)
{
    memoryAccessCallbacks.push_back(callback);
}

void Bus::setPpuOamTransferCallback(OamTransferCallback callback)
{
    ppuOamTransferCallback = callback;
}

void Bus::dispatchNmi()
{
    shouldDispatchNmi = true;
}

void Bus::dispatchOamTransfer()
{
    if (ppuOamTransferCallback)
    {
        // Will read from CPU memory $XX00 - $XXFF
        uint16_t address = read(PPU_OAMDATA) << 8;
        uint8_t *data = get(address);
        ppuOamTransferCallback(data);
    }

    shouldDispatchOamTransfer = false;
}

bool Bus::pollNmi()
{
    if (shouldDispatchNmi)
    {
        shouldDispatchNmi = false;
        return true;
    }

    return false;
}

bool Bus::pollOamTransfer()
{
    return shouldDispatchOamTransfer;
}

void Bus::setMapper(IMapper *mapper)
{
    this->mapper = mapper;
}

void Bus::dispatchMemoryAccessCallbacks(uint16_t address, uint8_t value,
                                        bool write)
{
    uint16_t target = address;

    if (address >= 0x0000 && address <= 0x1FFF)
    {
        // Get from CPU memory ($0000 - $1FFF, mirrored > $07FF)
        target = address % 0x0800;
    }
    else if (address >= 0x2000 && address <= 0x3FFF)
    {
        // Accessing PPU memory ($2000 - $3FFF, mirrored > $2007)
        // Normalize target to non-mirrored register range ($2000 - $2007)
        target = (address - 0x2000) % 0x0008 + 0x2000;
    }

    if (address == OAMDMA && write)
    {
        shouldDispatchOamTransfer = true;
    }

    for (auto &callback : memoryAccessCallbacks)
    {
        callback(target, value, write);
    }
}