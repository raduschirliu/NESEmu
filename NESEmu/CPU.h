#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "ROM.h"
#include "Memory.h"

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

	// Other components
	ROM *rom;
	Memory *memory;

	// Instruction table
	std::vector<Instruction> instructions;

	// Addressing modes (return true if possible to need an extra cycle)
	int IMP(), ACC(), IMM(), ZPG(), ZPX(), ZPY(), REL(),
		ABS(), ABX(), ABY(), IND(), IDX(), IDY();

	// Unknown or illegal instruction
	int XXX();

	// Instructions (return true if possible to need an extra cycle)
	int ADC(), AND(), ASL(), BCC(), BCS(), BEQ(), BIT(), BMI(), BNE(), BPL(), BRK(), BVC(), BVS(), CLC(),
		CLD(), CLI(), CLV(), CMP(), CPX(), CPY(), DEC(), DEX(), DEY(), EOR(), INC(), INX(), INY(), JMP(),
		JSR(), LDA(), LDX(), LDY(), LSR(), NOP(), ORA(), PHA(), PHP(), PLA(), PLP(), ROL(), ROR(), RTI(),
		RTS(), SBC(), SEC(), SED(), SEI(), STA(), STX(), STY(), TAX(), TAY(), TSX(), TXA(), TXS(), TYA();
};