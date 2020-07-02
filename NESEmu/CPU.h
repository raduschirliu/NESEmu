#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "ROM.h"
#include "Memory.h"
#include "Logger.h"

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
		int (CPU::*run)(void);
		int (CPU::*addressingMode)(void);
		uint8_t cycles;
	};

	CPU();

	// Step CPU by one single cycle
	void step();

	// Set status flag
	void setFlag(Flag flag);

	// Clear status flag
	void clearFlag(Flag flag);

	// Returns whether status flag is set or not
	bool hasFlag(Flag flag) const;

	// Sets memory pointer
	void setMemory(Memory *memory);

private:
	// Registers
	uint8_t a, x, y;
	uint8_t p, sp;
	uint16_t pc;

	// Used for current instruction
	uint8_t opcode;
	uint8_t *operand;
	uint8_t instructionLength;

	// Cycle related things
	uint8_t cycles;
	uint32_t totalCycles;

	// Other NES components
	ROM *rom;
	Memory *memory;
	Logger logger;

	// Instruction table
	std::vector<Instruction> instructions;

	// Sets overflow flag if result is an overflow
	void checkOverflow(int8_t target, int8_t result);

	// Sets carry flag if result requires a carry
	void checkCarry(int16_t result);

	// Sets negative flag if accumulator is negative
	void checkNegative(uint8_t target);

	// Sets zero flag if accumulator is zero
	void checkZero(uint8_t target);

	// Performs a branch and checks if page boundary is crossed
	int performBranch();

	// Addressing modes (return true if possible to need an extra cycle)
	int IMP(), ACC(), IMM(), ZPG(), ZPX(), ZPY(), REL(),
		ABS(), ABX(), ABY(), IND(), IDX(), IDY();

	// Unknown or illegal instruction (same functionality as NOP)
	int XXX();

	// Instructions (return true if possible to need an extra cycle)
	int ADC(), AND(), ASL(), BCC(), BCS(), BEQ(), BIT(), BMI(), BNE(), BPL(), BRK(), BVC(), BVS(), CLC(),
		CLD(), CLI(), CLV(), CMP(), CPX(), CPY(), DEC(), DEX(), DEY(), EOR(), INC(), INX(), INY(), JMP(),
		JSR(), LDA(), LDX(), LDY(), LSR(), NOP(), ORA(), PHA(), PHP(), PLA(), PLP(), ROL(), ROR(), RTI(),
		RTS(), SBC(), SEC(), SED(), SEI(), STA(), STX(), STY(), TAX(), TAY(), TSX(), TXA(), TXS(), TYA();
};
