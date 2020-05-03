#pragma once

#include <cstdint>

class CPU
{
public:
	CPU();

	enum Flags
	{
		Carry = 1 << 0,
		Zero = 1 << 1,
		Interrupt = 1 << 2,
		Decimal = 1 << 3,
		Break = 1 << 4,
		Unused = 1 << 5,
		Overflow = 1 << 6,
		Negative = 1 << 7
	};


private:
	uint8_t a, x, y;
	uint8_t p, sp;
	uint16_t pc;
	uint8_t opcode;
};