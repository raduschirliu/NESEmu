#include "Utils.h"

namespace utils
{
	void printMemory(std::stringstream &ss, uint16_t start, uint16_t end, std::function<uint8_t(uint16_t address)> readCallback)
	{
		// Draw 8 bytes per line from start address to end address
		for (int base = start; base <= end; base += 8)
		{
			ss << std::hex
				<< std::setw(4) << std::setfill('0')
				<< base << ":\t";

			for (int line = 0; line < 8; line++)
			{
				ss << std::setw(2) << std::setfill('0')
					<< (int)readCallback(base + line) << " ";
			}

			ss << std::endl;
		}
	}

	std::string mirroringModeToString(MirroringMode mode)
	{
		switch (mode)
		{
		case MirroringMode::HORIZONTAL: return "Horizontal";
		case MirroringMode::VERTICAL: return "Vertical";
		case MirroringMode::SINGLE_SCREEN: return "Single Screen";
		case MirroringMode::FOUR_SCREEN: return "4 Screen";
		case MirroringMode::CUSTOM: return "Custom";
		}

		return "Invalid";
	}
}