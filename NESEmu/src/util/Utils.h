#pragma once

#include <bitset>
#include <functional>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <type_traits>

#include "emulator/MirroringMode.h"

namespace utils
{

template <typename T,
          typename = typename std::enable_if_t<std::is_arithmetic<T>::value>>
std::string ToBitString(T num)
{
    const size_t size = std::numeric_limits<T>::digits;
    std::bitset<size> bits = num;
    return bits.to_string();
}

void PrintMemory(std::stringstream &ss, uint16_t start, uint16_t end,
                 std::function<uint8_t(uint16_t address)> readCallback);
std::string MirroringModeToString(MirroringMode mode);

}  // namespace utils