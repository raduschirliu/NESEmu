#pragma once

#include <cstdint>

// TODO: Move MirroringModeToString func inside of the MirroringMode class, and
// have the class take/keep a Value param
enum class MirroringMode : uint8_t
{
    kHorizontal = 0,
    kVertical,
    kSingleScreen,
    kFourScreen,
    kCustom
};
