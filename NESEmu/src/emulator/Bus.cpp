#include "Bus.h"

#include <iomanip>
#include <sstream>

// PPU register locations
static uint16_t constexpr PPU_OAMDATA = 0x4014;

Bus::Bus()
{
    // Allocate memory arrays for all NES components
    cpu_mem_ = new uint8_t[2048]();
    ppu_register_mem_ = new uint8_t[8]();
    apu_mem_ = new uint8_t[24]();
    test_mem_ = new uint8_t[8]();
    mapper_ = nullptr;

    // Other things needed on the bus
    should_dispatch_nmi_ = false;
    should_dispatch_oam_transfer_ = false;
}

Bus::~Bus()
{
    // Deallocate all memory arrays
    delete[] cpu_mem_;
    delete[] ppu_register_mem_;
    delete[] apu_mem_;
    delete[] test_mem_;
}

uint8_t *Bus::Get(uint16_t address)
{
    if (address >= 0x0000 && address <= 0x1FFF)
    {
        // Get from CPU memory ($0000 - $1FFF, mirrored > $07FF)
        uint16_t target = address % 0x0800;
        return &cpu_mem_[target];
    }
    else if (address >= 0x2000 && address <= 0x3FFF)
    {
        // Get from PPU register memory ($2000 - $3FFF, mirrored > $2007)
        uint16_t target = (address - 0x2000) % 0x0008;
        return &ppu_register_mem_[target];
    }
    else if (address >= 0x4000 && address <= 0x4017)
    {
        // Get from APU & I/O Memory ($4000 - $4017)
        return &apu_mem_[address - 0x4000];
    }
    else if (address >= 0x4018 && address <= 0x401F)
    {
        // Get from APU & I/O test Memory (usually disabled) ($4018 - $401F)
        return &test_mem_[address - 0x4018];
    }

    return nullptr;
}

uint8_t Bus::Read(uint16_t address, bool skip_callback)
{
    // Get from cartridge Memory ($4020 - $FFFF)
    if (address >= 0x4020 && address <= 0xFFFF)
    {
        return mapper_->PrgRead(address);
    }

    // Any other bus accesible memory (< $4020)
    uint8_t *ptr = Get(address);

    if (ptr != nullptr)
    {
        uint8_t val = *ptr;

        if (!skip_callback)
        {
            DispatchMemoryAccessCallbacks(address, 0, false);
        }

        return val;
    }

    // TODO: Return an error value?
    return 0;
}

void Bus::Write(uint16_t address, uint8_t value, bool skip_callback)
{
    // Get from cartridge Memory ($4020 - $FFFF)
    if (address >= 0x4020 && address <= 0xFFFF)
    {
        mapper_->PrgWrite(address, value);
        return;
    }

    // Any other bus accesible memory (< $4020)
    uint8_t *ptr = Get(address);

    if (ptr != nullptr)
    {
        *ptr = value;

        if (!skip_callback)
        {
            DispatchMemoryAccessCallbacks(address, value, true);
        }
    }
}

void Bus::Dump(Logger &logger)
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
            ss << std::setw(2) << std::setfill('0') << (int)Read(base + line)
               << " ";
        }

        ss << std::endl;
    }

    logger.Write(ss.str());
}

void Bus::RegisterMemoryAccessCallback(AccessCallback callback)
{
    memory_access_callbacks_.push_back(callback);
}

void Bus::PetPpuOamTransferCallback(OamTransferCallback callback)
{
    ppu_oam_transfer_callback_ = callback;
}

void Bus::DispatchNmi()
{
    should_dispatch_nmi_ = true;
}

void Bus::DispatchOamTransfer()
{
    if (ppu_oam_transfer_callback_)
    {
        // Will read from CPU memory $XX00 - $XXFF
        uint16_t address = Read(PPU_OAMDATA) << 8;
        uint8_t *data = Get(address);
        ppu_oam_transfer_callback_(data);
    }

    should_dispatch_oam_transfer_ = false;
}

bool Bus::PollNmi()
{
    if (should_dispatch_nmi_)
    {
        should_dispatch_nmi_ = false;
        return true;
    }

    return false;
}

bool Bus::PollOamTransfer()
{
    return should_dispatch_oam_transfer_;
}

void Bus::SetMapper(IMapper *mapper)
{
    this->mapper_ = mapper;
}

void Bus::DispatchMemoryAccessCallbacks(uint16_t address, uint8_t value,
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
        should_dispatch_oam_transfer_ = true;
    }

    for (auto &callback : memory_access_callbacks_)
    {
        callback(target, value, write);
    }
}