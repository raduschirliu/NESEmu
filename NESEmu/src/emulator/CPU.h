#pragma once

#include "ROM.h"
#include "Memory.h"
#include "../debug/Logger.h"

#include <cstdint>
#include <string>
#include <vector>

// Handles emulation of the NES 6502 CPU
class CPU
{
public:
	// An emun represantation of the 8 status flags used for the 6502's status (p) register
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

	// A representation of the an instruction and it's associated addressing mode
	struct Instruction
	{
		std::string instruction;
		int (CPU:: *run)(void);
		int (CPU:: *addressingMode)(void);
		uint8_t cycles;
	};

	// Represents current CPU state, used for debugging
	struct State
	{
		// Registers, PC, and total cycles
		uint8_t a, x, y, p, sp;
		uint16_t pc;
		uint32_t totalCycles;
		uint8_t instructionLength;

		// Current instruction
		uint8_t opcode, cycles;
		std::string instruction = "";
		std::string addressingMode = "";
	} state;

	// Initialize CPU
	CPU(Memory &memory);

	// Step CPU by one single cycle
	void step();

	// Set status flag
	void setFlag(Flag flag);

	// Clear status flag
	void clearFlag(Flag flag);

	// Set single status bit value (value MUST be 0/1)
	void setStatusBit(uint8_t bit, uint8_t value);

	// Set single status flag value (value MUST be 0/1)
	void setFlagValue(Flag flag, uint8_t value);

	// Returns whether status flag is set or not
	bool hasFlag(Flag flag) const;

	// Sets program counter
	void setPC(uint16_t pc);

	// Returns current state of CPU and registers
	State getState() const;

private:
	// Registers
	uint8_t a, x, y;
	uint8_t p, sp;
	uint16_t pc;

	// Used for current instruction
	uint8_t opcode;
	uint8_t *operand;
	uint8_t instructionLength;
	uint16_t jumpTarget;

	// Cycle related stats
	uint8_t cycles;
	uint32_t totalCycles;

	// Other NES components, external to the CPU
	// ROM &rom;
	Memory &memory;

	// Logger for debugging
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
