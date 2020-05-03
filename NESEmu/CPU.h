#pragma once

#include <cstdint>
#include <string>
#include <vector>

class CPU
{
public:
	enum class Flag : uint8_t
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

	struct Instruction
	{
		std::string instruction;
		bool (CPU::*run)(void);
		bool (CPU::*addressingMode)(void);
		uint8_t cycles;
	};

	CPU();
	void step();
	void setFlag(Flag flag);
	bool hasFlag(Flag flag) const;

private:
	// Registers
	uint8_t a, x, y;
	uint8_t p, sp;
	uint16_t pc;

	// Current opcode
	uint8_t opcode;

	// Cycle related things
	uint8_t cycles;
	uint32_t totalCycles;

	// Instruction table
	std::vector<Instruction> instructions;

	// Addressing modes
	bool IMP(), ACC(), IMM(), ZPP(), ZPX(), ZPY(), REL(),
		ABS(), ABX(), ABY(), IND(), IDX(), IDY();

	// Instructions
};