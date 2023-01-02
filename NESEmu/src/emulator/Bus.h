#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include "IMapper.h"
#include "util/Logger.h"

// Contains the 64 KiB of RAM that is addressable by the 6502 CPU
class Bus
{
  public:
    // All mapped register addresses
    static constexpr uint16_t PPUCTRL = 0x2000;
    static constexpr uint16_t OAMDMA = 0x4014;
    static constexpr uint16_t JOY1 = 0x4016;
    static constexpr uint16_t JOY2 = 0x4017;

    // Addressess
    static constexpr uint16_t CARTRIDGE_ADDRESS = 0x4020;

    // Callback types
    using AccessCallback =
        std::function<void(uint16_t address, uint8_t new_value, bool write)>;
    using OamTransferCallback = std::function<void(uint8_t *data)>;

    // Initialize all empty memory blocks
    Bus();

    // Deallocate all memory
    ~Bus();

    // Return pointer to a place in memory, or nullptr if out of bounds
    uint8_t *Get(uint16_t address);

    // Return value of a place in memory, or -1 if not found
    uint8_t Read(uint16_t address, bool skip_callback = false);

    // Write value in memory at address, or does nothing if out of bounds
    void Write(uint16_t address, uint8_t value, bool skip_callback = false);

    // Dump entire contents of memory ($0000 - $FFFF) to given logger
    void Dump(Logger &logger);

    // Register a new memory access callback
    void RegisterMemoryAccessCallback(AccessCallback callback);

    // Set the PPU Oam transfer callback
    void PetPpuOamTransferCallback(OamTransferCallback callback);

    // Signal that the NMI should be dispatched to the CPU
    void DispatchNmi();

    // Signal to the PPU that the OAM data transfer should begin
    void DispatchOamTransfer();

    // Poll for whether the NMI should be dispatched
    bool PollNmi();

    // Poll for whether an OAM data transfer is requested
    bool PollOamTransfer();

    // Sets the active mapper
    void SetMapper(IMapper *mapper);

  private:
    // Internal 2 KiB of CPU Memory (from $0000 - $07FFF)
    // Mirrored 3 times from $0800 - $1FFF
    uint8_t *cpu_mem_;

    // PPU Memory mapped registers (from $2000 - $2007)
    // Mirrored every 8 bytes from $2008 - $3FFF
    uint8_t *ppu_register_mem_;

    // APU & I/O Memory (from $4000 - $4017)
    uint8_t *apu_mem_;

    // APU & I/O Memory - usually disabled (from $4018 - $401F)
    uint8_t *test_mem_;

    // Cartridge Memory: used for PRG ROM, PRG RAM, and mapper registers (from
    // $4020 - $FFFF)
    IMapper *mapper_;

    // Callbacks
    std::vector<AccessCallback> memory_access_callbacks_;
    OamTransferCallback ppu_oam_transfer_callback_;

    // Whether the NMI has already been dispatched to the CPU
    bool should_dispatch_nmi_;

    // Whether an OAM transfer is needed
    bool should_dispatch_oam_transfer_;

    // Dispatch any memory access callbacks if needed
    void DispatchMemoryAccessCallbacks(uint16_t address, uint8_t value,
                                       bool write);
};