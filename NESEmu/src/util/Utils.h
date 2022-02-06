#pragma once

#include "../emulator/MirroringMode.h"

#include <type_traits>
#include <string>
#include <bitset>
#include <limits>
#include <sstream>
#include <iomanip>
#include <functional>

namespace utils
{
	template <typename T, typename = typename std::enable_if_t<std::is_arithmetic<T>::value >>
	std::string toBitString(T num)
	{
		const size_t size = std::numeric_limits<T>::digits;
		std::bitset<size> bits = num;
		return bits.to_string();
	}

	void printMemory(std::stringstream &ss, uint16_t start, uint16_t end, std::function<uint8_t(uint16_t address)> readCallback);
	std::string mirroringModeToString(MirroringMode mode);
}