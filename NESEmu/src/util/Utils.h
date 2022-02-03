#pragma once

#include <type_traits>
#include <string>
#include <bitset>
#include <limits>

namespace utils
{
	template <typename T, typename = typename std::enable_if_t<std::is_arithmetic<T>::value >>
	std::string toBitString(T num)
	{
		const size_t size = std::numeric_limits<T>::digits;
		std::bitset<size> bits = num;
		return bits.to_string();
	}
}