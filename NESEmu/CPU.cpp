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
}

void CPU::step()
{
	totalCycles++;
}

void CPU::setFlag(Flag flag)
{
	p |= (uint8_t)flag;
}

bool CPU::hasFlag(Flag flag) const
{
	return (p & (uint8_t)flag) > 0;
}