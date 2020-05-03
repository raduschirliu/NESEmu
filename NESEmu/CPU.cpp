#include "CPU.h"

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

	};
}

// Step CPU by one cycle
void CPU::step()
{
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