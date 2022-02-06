#pragma once

#include <cstdint>

enum class MirroringMode : uint8_t
{
	HORIZONTAL = 0,
	VERTICAL,
	SINGLE_SCREEN,
	FOUR_SCREEN,
	CUSTOM
};
