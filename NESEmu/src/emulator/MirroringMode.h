#pragma once

#include <cstdint>

enum class MirroringMode : uint8_t
{
    kHorizontal = 0,
    kVertical,
    kSingleScreen,
    kFourScreen,
    kCustom
};
