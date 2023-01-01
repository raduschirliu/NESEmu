#include "Utils.h"

namespace utils
{

void PrintMemory(std::stringstream &ss, uint16_t start, uint16_t end,
                 std::function<uint8_t(uint16_t address)> read_callback)
{
    // Draw 8 bytes per line from start address to end address
    for (int base = start; base <= end; base += 8)
    {
        ss << std::hex << std::setw(4) << std::setfill('0') << base << ":\t";

        for (int line = 0; line < 8; line++)
        {
            ss << std::setw(2) << std::setfill('0')
               << (int)read_callback(base + line) << " ";
        }

        ss << std::endl;
    }
}

std::string MirroringModeToString(MirroringMode mode)
{
    switch (mode)
    {
        case MirroringMode::kHorizontal:
            return "Horizontal";
        case MirroringMode::kVertical:
            return "Vertical";
        case MirroringMode::kSingleScreen:
            return "Single Screen";
        case MirroringMode::kFourScreen:
            return "4 Screen";
        case MirroringMode::kCustom:
            return "Custom";
    }

    return "Invalid";
}

}  // namespace utils