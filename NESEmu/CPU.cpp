#include <stdio.h>
#include "CPU.h"

// Convenience macros for defining CPU instructions
#define _I(NAME, RUN, MODE, CYCLES) { NAME, &CPU::RUN, &CPU::MODE, CYCLES }
#define _XXX() { "XXX", &CPU::XXX, &CPU::IMP, 2 }

// Initialize CPU
CPU::CPU()
{
	a = 0x00;
	x = 0x00;
	y = 0x00;
	p = 0x00;
	pc = 0x0000;
	sp = 0x01FF;
	opcode = 0x00;
	totalCycles = 0;
	cycles = 0;

	// Initialize instruction table
	instructions =
	{
		_I("BRK", BRK, IMP, 7), _I("ORA", ORA, IDX, 6), _XXX(),					_XXX(), _XXX(),					_I("ORA", ORA, ZPG, 3), _I("ASL", ASL, ZPG, 5), _XXX(), _I("PHP", PHP, IMP, 3), _I("ORA", ORA, IMM, 2),	_I("ASL", ASL, ACC, 2), _XXX(), _XXX(),					_I("ORA", ORA, ABS, 4), _I("ASL", ASL, ABS, 6), _XXX(),
		_I("BPL", BPL, REL, 2), _I("ORA", ORA, IDY, 5), _XXX(),					_XXX(), _XXX(),					_I("ORA", ORA, ZPX, 4), _I("ASL", ASL, ZPX, 6), _XXX(), _I("CLC", CLC, IMP, 2), _I("ORA", ORA, ABY, 4),	_XXX(),					_XXX(), _XXX(),					_I("ORA", ORA, ABX, 4), _I("ASL", ASL, ABX, 7), _XXX(),
		_I("JSR", JSR, ABS, 6), _I("AND", AND, IDX, 6), _XXX(),					_XXX(), _I("BIT", BIT, ZPG, 3), _I("AND", AND, ZPG, 3), _I("ROL", ROL, ZPG, 5), _XXX(), _I("PLP", PLP, IMP, 4), _I("AND", AND, IMM, 2), _I("ROL", ROL, ACC, 2), _XXX(), _I("BIT", BIT, ABS, 4), _I("AND", AND, ABS, 4), _I("ROL", ROL, ABS, 6), _XXX(),
		_I("BMI", BMI, REL, 2), _I("AND", AND, IDY, 5), _XXX(),					_XXX(), _XXX(),					_I("AND", AND, ZPX, 4), _I("ROL", ROL, ZPX, 6), _XXX(), _I("SEC", SEC, IMP, 2), _I("AND", AND, ABY, 4), _XXX(),					_XXX(), _XXX(),					_I("AND", AND, ABX, 4), _I("ROL", ROL, ABX, 7), _XXX(),
		_I("RTI", RTI, IMP, 6), _I("EOR", EOR, IDX, 6), _XXX(),					_XXX(), _XXX(),					_I("EOR", EOR, ZPG, 3), _I("LSR", LSR, ZPG, 5), _XXX(), _I("PHA", PHA, IMP, 3), _I("EOR", EOR, IMM, 2), _I("LSR", LSR, ACC, 2), _XXX(), _I("JMP", JMP, ABS, 3), _I("EOR", ABS, ABS, 4), _I("LSR", LSR, ABS, 6), _XXX(),
		_I("BVC", BVC, REL, 2), _I("EOR", EOR, IDY, 5), _XXX(),					_XXX(), _XXX(),					_I("EOR", EOR, ZPX, 4), _I("LSR", LSR, ZPX, 6), _XXX(), _I("CLI", CLI, IMP, 2), _I("EOR", EOR, ABY, 4), _XXX(),					_XXX(), _XXX(),					_I("EOR", EOR, ABX, 4), _I("LSR", LSR, ABX, 7), _XXX(),
		_I("RTS", RTS, IMP, 6), _I("ADC", ADC, IDX, 6), _XXX(),					_XXX(), _XXX(),					_I("ADC", ADC, ZPG, 3), _I("ROR", ROR, ZPG, 5), _XXX(), _I("PLA", PLA, IMP, 4), _I("ADC", ADC, IMM, 2), _I("ROR", ROR, ACC, 2), _XXX(), _I("JMP", JMP, IND, 5), _I("ADC", ADC, ABS, 4), _I("ROR", ROR, ABS, 4), _XXX(),
		_I("BVS", BVS, REL, 2), _I("ADC", ADC, IDY, 5), _XXX(),					_XXX(), _XXX(),					_I("ADC", ADC, ZPX, 4), _I("ROR", ROR, ZPX, 6), _XXX(), _I("SEI", SEI, IMP, 2), _I("ADC", ADC, ABY, 4), _XXX(),					_XXX(), _XXX(),					_I("ADC", ADC, ABX, 4), _I("ROR", ROR, ABX, 7), _XXX(),
		_XXX(),					_I("STA", STA, IDX, 6), _XXX(),					_XXX(), _I("STY", STY, ZPG, 3), _I("STA", STA, ZPG, 3), _I("STX", STX, ZPG, 3), _XXX(), _I("DEY", DEY, IMP, 2), _XXX(),					_I("TXA", TXA, IMP, 2), _XXX(), _I("STY", STY, ABS, 4), _I("STA", STA, ABS, 4), _I("STX", STX, ABS, 4), _XXX(),
		_I("BCC", BCC, REL, 2), _I("STA", STA, IDY, 6), _XXX(),					_XXX(), _I("STY", STY, ZPX, 4), _I("STA", STA, ZPX, 4), _I("STX", STX, ZPY, 4), _XXX(), _I("TYA", TYA, IMP, 2), _I("STA", STA, ABY, 5), _I("TXS", TXS, IMP, 2), _XXX(), _XXX(),					_I("STA", STA, ABX, 5), _XXX(),					_XXX(),
		_I("LDY", LDY, IMM, 2), _I("LDA", LDA, IDX, 6), _I("LDX", LDX, IMM, 2), _XXX(), _I("LDY", LDY, ZPG, 3), _I("LDA", LDA, ZPG, 3), _I("LDX", LDX, ZPG, 3), _XXX(), _I("TAY", TAY, IMP, 2), _I("LDA", LDA, IMM, 2), _I("TAX", TAX, IMP, 2), _XXX(), _I("LDY", LDY, ABS, 4), _I("LDA", LDA, ABS, 4), _I("LDX", LDX, ABS, 4), _XXX(),
	};
}

// Step CPU by one cycle
void CPU::step()
{
	if (cycles > 0)
	{
		cycles--;
	}
	else
	{
		// Process opcode
		Instruction ins = instructions[opcode];
		int modeExtra = (this->*ins.addressingMode)();
		int runExtra = (this->*ins.run)();

		pc++;
		cycles += ins.cycles;

		if (modeExtra > 0 && runExtra > 0)
		{
			cycles += runExtra;
		}
	}

	totalCycles++;
}

// Set status flag
void CPU::setFlag(Flag flag)
{
	p |= (uint8_t)flag;
}

// Get whether status flag has been sets
bool CPU::hasFlag(Flag flag) const
{
	return (p & (uint8_t)flag) > 0;
}


/* Addressing Modes */

// Implicit or implied
int IMP()
{
	
}

// Accumulator
int ACC()
{
	
}

// Immediate
int IMM()
{

}

// Zero-page
int ZPG()
{

}

// Zero-page, X
int ZPX()
{

}

// Zero-page, Y
int ZPY()
{

}

// Relative
int REL()
{

}

// Absolute
int ABS()
{

}

// Absolute, X
int ABX()
{

}

// Absolute, Y
int ABY()
{

}

// Indirect
int IND()
{

}

// Indirect, X
int IDX()
{

}

// Indirect, Y
int IDY()
{

}


/* Instructions */

// Unknown or illegal instruction, do nothing (same functionality as NOP)
int XXX()
{
	return 0;
}