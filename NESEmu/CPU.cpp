#include <stdio.h>
#include "CPU.h"

// Debugging
#define DEBUG_CONSOLE false
#define DEBUG_LOG true

// Convenience macros for defining CPU instructions
#define _I(NAME, RUN, MODE, CYCLES) { NAME, &CPU::RUN, &CPU::MODE, CYCLES }
#define _XXX() { "XXX", &CPU::XXX, &CPU::IMP, 2 }

// Initialize CPU
CPU::CPU(Memory *memory) : logger("C:\\Dev\\Projects\\nesemu\\logs\\cpu.txt"), memory(memory)
{
	if (memory == nullptr)
	{
		printf("Invalid memory pointer provided to CPU\n");
		return;
	}

	a = 0x00;
	x = 0x00;
	y = 0x00;
	p = 0x00;
	pc = 0x0000;
	sp = 0x01FF;
	opcode = 0x00;
	totalCycles = 0;
	cycles = 0;

	// TEST
	p = 0x24;
	sp = 0xFD;

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
		_I("BCS", BCS, REL, 2), _I("LDA", LDA, IDY, 5), _XXX(),					_XXX(), _I("LDY", LDY, ZPX, 4), _I("LDA", LDA, ZPX, 4), _I("LDX", LDX, ZPY, 4), _XXX(), _I("CLV", CLV, IMP, 2), _I("LDA", LDA, ABY, 4), _I("TSX", TSX, IMP, 2), _XXX(), _I("LDY", LDY, ABX, 4), _I("LDA", LDA, ABX, 4), _I("LDX", LDX, ABY, 4), _XXX(),
		_I("CPY", CPY, IMM, 2), _I("CMP", CMP, IDX, 6), _XXX(),					_XXX(), _I("CPY", CPY, ZPG, 3), _I("CMP", CMP, ZPG, 3), _I("DEC", DEC, ZPG, 5), _XXX(), _I("INY", INY, IMP, 2), _I("CMP", CMP, IMM, 2), _I("DEX", DEX, IMP, 2), _XXX(), _I("CPY", CPY, ABS, 4), _I("CMP", CMP, ABS, 4), _I("DEC", DEC, ABS, 6), _XXX(),
		_I("BNE", BNE, REL, 2), _I("CMP", CMP, IDY, 5), _XXX(),					_XXX(), _XXX(),					_I("CMP", CMP, ZPX, 4), _I("DEC", DEC, ZPX, 6), _XXX(), _I("CLD", CLD, IMP, 2), _I("CMP", CMP, ABY, 4), _XXX(),					_XXX(),	_XXX(),					_I("CMP", CMP, ABX, 4), _I("DEC", DEC, ABX, 7), _XXX(),
		_I("CPX", CPX, IMM, 2), _I("SBC", SBC, IDX, 6), _XXX(),					_XXX(), _I("CPX", CPX, ZPG, 3), _I("SBC", SBC, ZPG, 3), _I("INC", INC, ZPG, 5), _XXX(), _I("INX", INX, IMP, 2), _I("SBC", SBC, IMM, 2), _I("NOP", NOP, IMP, 2), _XXX(), _I("CPX", CPX, ABS, 4), _I("SBC", SBC, ABS, 4), _I("INC", INC, ABS, 6), _XXX(),
		_I("BEQ", BEQ, REL, 2), _I("SBC", SBC, IDY, 5), _XXX(),					_XXX(), _XXX(),					_I("SBC", SBC, ZPX, 4), _I("INC", INC, ZPX, 6), _XXX(), _I("SED", SED, IMP, 2), _I("SBC", SBC, ABY, 4), _XXX(),					_XXX(),	_XXX(),					_I("SBC", SBC, ABX, 4), _I("INC", INC, ABX, 7), _XXX()
	};
}

void CPU::step()
{
	if (cycles > 0)
	{
		if (DEBUG_CONSOLE)
		{
			printf("Stepping...\n");
			printf("PC: %X\n", pc);
			printf("CYCLE: %d\n\n", totalCycles);
		}
		
		cycles--;
	}
	else
	{
		// Process opcode
		opcode = memory->read(pc);
		Instruction ins = instructions[opcode];
		instructionLength = 1;
		int modeExtra = (this->*ins.addressingMode)();

		// ---- Debug
		if (DEBUG_CONSOLE)
		{
			printf("Pre-Execution\n");
			printf("------------------------------\n");
			printf("INSTR: %s\nPC: %X\nOPCODE: %X\n", ins.instruction.c_str(), pc, opcode);
			printf("LENGTH: %d\nCYCLES: %d\n", instructionLength, ins.cycles);

			if (operand == nullptr)
			{
				printf("OPERAND: nullptr\n");
			}
			else
			{
				printf("OPERAND: %X\n", *operand);
			}

			printf("STATUS (NO-BDIZC): ");

			for (int i = 0; i < 8; i++)
			{
				uint8_t bit = ((p >> 7) - i) & 0b00000001;
				printf("%d", bit);
			}

			printf(" | $%X\n", p);

			printf("A: %d | $%X\n", a, a);
			printf("X: %d | $%X\n", x, x);
			printf("Y: %d | $%X\n", y, y);
			printf("SP: %d | $%X\n", sp, sp);
			printf("CYCLE: %d\n", totalCycles);
			printf("------------------------------\n\n");
		}

		if (DEBUG_LOG)
		{
			char buf[500];
			sprintf_s(buf, "%04X  %02X  %s\t\tA:%02X X:%02X Y:%02X P:%02X SP:%02X\tCYC:%d\n", pc, opcode, ins.instruction.c_str(), a, x, y, p, sp, totalCycles);
			logger.write(buf);
		}
		// ---- Debug

		int runExtra = (this->*ins.run)();

		// TODO: Fix cycle calculation, runs behind
		pc += instructionLength;
		cycles += ins.cycles;

		if (modeExtra > 0 && runExtra > 0)
		{
			cycles += runExtra;
		}

		cycles--;
	}

	totalCycles++;
}

void CPU::setFlag(Flag flag)
{
	p |= (uint8_t)flag;
}

void CPU::clearFlag(Flag flag)
{
	p &= ~(uint8_t)flag;
}

bool CPU::hasFlag(Flag flag) const
{
	return (p & (uint8_t)flag) > 0;
}

void CPU::setPC(uint16_t pc)
{
	this->pc = pc;
}

void CPU::checkOverflow(int8_t target, int8_t result)
{
	int8_t signedOperand = *operand;

	if (target > 0 && signedOperand > 0 && result < 0)
	{
		setFlag(Flag::Overflow);
	}
	else if (target < 0 && signedOperand < 0 && result > 0)
	{
		setFlag(Flag::Overflow);
	}
	else
	{
		clearFlag(Flag::Overflow);
	}
}

void CPU::checkCarry(int16_t value)
{
	if (value > 127 || value < -128)
	{
		setFlag(Flag::Carry);
	}
	else
	{
		clearFlag(Flag::Carry);
	}
}

void CPU::checkNegative(uint8_t target)
{
	if ((target & 0b10000000) > 0)
	{
		setFlag(Flag::Negative);
	}
	else
	{
		clearFlag(Flag::Negative);
	}
}

void CPU::checkZero(uint8_t target)
{
	if (target == 0)
	{
		setFlag(Flag::Zero);
	}
	else
	{
		clearFlag(Flag::Zero);
	}
}

int CPU::performBranch()
{
	// Update the PC
	int8_t offset = *operand;
	pc += offset;

	// TODO: Check if branch occurs on same page or different page
	return 1;
}

/* Addressing Modes */

// Implicit or implied
int CPU::IMP()
{
	// No operand
	operand = nullptr;

	return 0;
}

// Accumulator
int CPU::ACC()
{
	// Operand is accumulator
	operand = &a;

	return 0;
}

// Immediate
int CPU::IMM()
{
	// Operand given in 1 byte after instruction
	operand = memory->get(pc + 1);
	instructionLength++;

	return 0;
}

// Zero-page
int CPU::ZPG()
{
	// Operand is a memory address in range $0000-$00FF, in 1 byte after instruction
	uint16_t address = memory->read(pc + 1);
	operand = memory->get(address);
	instructionLength++;

	/*
	char buff[500];
	sprintf_s(buff, "\tMemory address: %02X\n\tMemory address value: %02X\n", address, *operand);
	logger.write(buff);
	*/

	return 0;
}

// Zero-page, X
int CPU::ZPX()
{
	// Operand is a memory address in range $0000-$00FF added to X register
	uint16_t address = memory->read(pc + 1) + x;
	operand = memory->get(address);
	instructionLength++;

	// TODO: Take zero page wrap-around into account

	return 0;
}

// Zero-page, Y
int CPU::ZPY()
{
	// Operand is a memory address in range $0000-$00FF added to Y register
	uint16_t address = memory->read(pc + 1) + y;
	operand = memory->get(address);
	instructionLength++;

	return 0;
}

// Relative (Only used by branch) [+]
int CPU::REL()
{
	// Operand is a SIGNED bit after the instruction
	operand = memory->get(pc + 1);
	instructionLength++;

	return 1;
}

// Absolute
int CPU::ABS()
{
	// Operand is an address after the instruction, stored in little-endian
	uint16_t address = memory->read(pc + 2) << 8;
	address |= memory->read(pc + 1);
	operand = memory->get(address);

	// Jump target is the 16-bit address after the instruction
	jumpTarget = address;

	instructionLength += 2;

	return 0;
}

// Absolute, X [+]
int CPU::ABX()
{
	// Operand is an address after the instruction, stored in little-endian, added to X
	uint16_t address = memory->read(pc + 2) << 8;
	address |= memory->read(pc + 1);
	operand = memory->get(address + x);
	instructionLength += 2;

	// TODO: Deal with carry
	// TODO: Check if violating page boundary
	return 1;
}

// Absolute, Y [+]
int CPU::ABY()
{
	// Operand is an address after the instruction, stored in little-endian, added to Y
	uint16_t address = memory->read(pc + 2) << 8;
	address |= memory->read(pc + 1);
	operand = memory->get(address + y);
	instructionLength += 2;

	// TODO: Deal with carry
	// TODO: Check if violating page boundary
	return 1;
}

// Indirect (only used by JMP)
int CPU::IND()
{
	// Jump target is a 16-bit value at address specified after the instruction, stored in little-endian
	uint16_t address = memory->read(pc + 2) << 8;
	address |= memory->read(pc + 1);

	jumpTarget = memory->read(address + 1) << 8;
	jumpTarget |= memory->read(address);

	// Operand unused
	operand = nullptr;
	instructionLength += 2;

	return 0;
}

// Indirect, X
int CPU::IDX()
{
	// TODO: Test this

	// Read the pointer address after the instruction and add X to it
	uint16_t pointer = memory->read(pc + 2) << 8;
	pointer |= memory->read(pc + 1);
	pointer += x;

	// Read the address located at the pointer
	uint16_t address = memory->read(pc + 2) << 8;
	address |= memory->read(pc + 1);

	operand = memory->get(address);
	instructionLength += 2;

	// TODO: Take zero page wrap-around into account
	return 0;
}

// Indirect, Y [+]
int CPU::IDY()
{
	// TODO: Test this

	// Read the pointer address after the instruction
	uint16_t pointer = memory->read(pc + 2) << 8;
	pointer |= memory->read(pc + 1);

	// Read the address located at the pointer, and add Y to it
	uint16_t address = memory->read(pc + 2) << 8;
	address |= memory->read(pc + 1);
	address += y;

	operand = memory->get(address);
	instructionLength += 2;

	return 1;
}


/* Instructions */

// Unknown or illegal instruction, do nothing (same functionality as NOP)
int CPU::XXX()
{
	return 0;
}

// A + M + C -> A, C (NZCV); Add with carry
int CPU::ADC()
{
	uint16_t result = a + *operand + hasFlag(Flag::Carry);

	checkNegative(result);
	checkZero(result);
	checkOverflow(a, result);

	if (result > 0xFF)
	{
		setFlag(Flag::Carry);
	}
	else
	{
		clearFlag(Flag::Carry);
	}

	a = result;

	// ABX, ABY, IDY add one cycle if page boundary crossed
	return 1;
}

// A & M -> A (NZ); Logical AND on accumulator and memory
int CPU::AND()
{
	a &= *operand;

	checkNegative(a);
	checkZero(a);

	// ABX, ABY, IDY addressing modes add one cycle if page boundary crossed
	return 1;
}

// M << 1 -> M; (NZC); Shift left one bit
int CPU::ASL()
{
	uint16_t result = *operand << 1;
	
	checkNegative(result);
	checkZero(result);
	
	if (result > 0xFF)
	{
		setFlag(Flag::Carry);
	}
	else
	{
		clearFlag(Flag::Carry);
	}

	return 0;
}

// C ?= 0 -> Branch; (); Branch on carry clear
int CPU::BCC()
{
	if (!hasFlag(Flag::Carry))
	{
		return performBranch();
	}

	return 0;
}

// C ?= 1 -> Branch; (); Branch on carry set
int CPU::BCS()
{
	if (hasFlag(Flag::Carry))
	{
		return performBranch();
	}

	return 0;
}

// Z ?= 1 -> Branch; (); Branch on result zero
int CPU::BEQ()
{
	if (hasFlag(Flag::Zero))
	{
		return performBranch();
	}

	return 0;
}

// op6 -> V, op7 -> N, A and M; (NZV); Operand bits 6/7 transfered to status bits 6/7. Operand anded with accumulator
int CPU::BIT()
{
	// Bits 7 and 6 transferred to the status register
	uint8_t bit7 = !!(*operand >> 7);
	p ^= (-bit7 ^ p) & (1UL << 7);

	uint8_t bit6 = !!((*operand >> 6) & 0b01);
	p ^= (-bit6 ^ p) & (1UL << 6);

	/*
	char buff[500];
	sprintf_s(buff, "\tOperand: %02X\n\tBit 7: %02X\n\tBit 6: %02X\n\tacc & op: %02X\n", *operand, bit7, bit6, (a & *operand));
	logger.write(buff);
	*/

	// Operand and accumulator ANDed to get zero flag value
	if ((a & *operand) == 0)
	{
		setFlag(Flag::Zero);
	}
	else
	{
		clearFlag(Flag::Zero);
	}

	return 0;
}

// N ?= 1 -> Branch; (); Branch on result negative
int CPU::BMI()
{
	if (hasFlag(Flag::Negative))
	{
		return performBranch();
	}

	return 0;
}

// Z ?= 0 -> Branch; (); Branch on result not zero
int CPU::BNE()
{
	if (!hasFlag(Flag::Zero))
	{
		return performBranch();
	}

	return 0;
}

// N ?= 0 -> Branch; (); Branch on result positive
int CPU::BPL()
{
	if (!hasFlag(Flag::Negative))
	{
		return performBranch();
	}

	return 0;
}

// ; (I); Force interrupt, push PC+2 and status pointer
int CPU::BRK()
{
	memory->set(sp - 1, pc >> 8); // PC high byte
	memory->set(sp - 2, pc & 0x00FF); // PC low byte
	memory->set(sp - 3, p); // Status
	
	setFlag(Flag::Interrupt);

	sp -= 3;

	return 0;
}

// V ?= 0 -> Branch; (); Branch on overflow clear
int CPU::BVC()
{
	if (!hasFlag(Flag::Overflow))
	{
		return performBranch();
	}

	return 0;
}

// V = 1 -> Branch; (); Branch on overflow set
int CPU::BVS()
{
	if (hasFlag(Flag::Overflow))
	{
		return performBranch();
	}

	return 0;
}

// 0 -> C; (C); Clear carry flag
int CPU::CLC()
{
	clearFlag(Flag::Carry);
	return 0;
}

// 0 -> D; (D); Clear decimal flag
int CPU::CLD()
{
	clearFlag(Flag::Decimal);
	return 0;
}

// 0 -> I; (I); Clear interrupt flag
int CPU::CLI()
{
	clearFlag(Flag::Interrupt);
	return 0;
}

// 0 -> V; (V); Clear overflow flag
int CPU::CLV()
{
	clearFlag(Flag::Overflow);
	return 0;
}

// A - M; (NZC); Compare memory with accumulator
int CPU::CMP()
{
	uint8_t result = a - *operand;

	//printf("A: %d\nM: %d\nR: %d\n", a, *operand, result);

	checkNegative(result);
	checkZero(result);
	
	if (a >= *operand)
	{
		setFlag(Flag::Carry);
	}
	else
	{
		clearFlag(Flag::Carry);
	}

	// ABX, ABY, IDY add one cycle if page boundary crossed
	return 1;
}

// X - M; (NZC); Compare memory with X
int CPU::CPX()
{
	uint8_t result = x - *operand;

	checkNegative(result);
	checkZero(result);
	
	if (x >= *operand)
	{
		setFlag(Flag::Carry);
	}
	else
	{
		clearFlag(Flag::Carry);
	}

	// ABX, ABY, IDY add one cycle if page boundary crossed
	return 1;
}

// Y - M; (NZC); Comapre memory with Y
int CPU::CPY()
{
	uint8_t result = y - *operand;

	checkNegative(result);
	checkZero(result);
	
	if (y >= *operand)
	{
		setFlag(Flag::Carry);
	}
	else
	{
		clearFlag(Flag::Carry);
	}

	// ABX, ABY, IDY add one cycle if page boundary crossed
	return 1;
}

// M - 1 -> M; (NZ); Decrement memory by one
int CPU::DEC()
{
	(*operand)--;

	checkNegative(*operand);
	checkZero(*operand);

	return 0;
}

// X - 1 -> X; (NZ); Decrement X register by one
int CPU::DEX()
{
	x--;

	checkNegative(x);
	checkZero(x);

	return 0;
}

// Y - 1 -> Y; (NZ); Decrement Y register by one
int CPU::DEY()
{
	y--;

	checkNegative(y);
	checkZero(y);

	return 0;
}

// A XOR M -> A; (NZ); Exclusive OR (XOR) memory with accumulator
int CPU::EOR()
{
	a = a ^ *operand;

	checkNegative(a);
	checkZero(a);

	// ABX, ABY, IDY require extra cycle if page boundary crossed
	return 1;
}

// M + 1 -> M; (NZ); Increment memory by one
int CPU::INC()
{
	(*operand)++;

	checkNegative(*operand);
	checkZero(*operand);

	return 0;
}

// X + 1 -> X; (NZ); Increment X by one
int CPU::INX()
{
	x++;

	checkNegative(x);
	checkZero(x);

	return 0;
}

// Y + 1 -> Y; (NZ); Increment Y by one
int CPU::INY()
{
	y++;

	checkNegative(y);
	checkZero(y);

	return 0;
}

// Target -> PC; (); Jump to new location
int CPU::JMP()
{
	pc = jumpTarget - instructionLength;
	return 0;
}

// Push PC + 2, Target -> PC; (); Jump to new location, saving return address
int CPU::JSR()
{
	uint16_t returnAddress = pc + instructionLength;
	memory->set(sp - 1, (returnAddress & 0xFF00) >> 8);
	memory->set(sp - 2, returnAddress & 0x00FF);
	sp -= 2;

	pc = jumpTarget - instructionLength;

	return 0;
}

// M -> A; (NZ); Load accumulator with memory
int CPU::LDA()
{
	a = *operand;

	checkNegative(a);
	checkZero(a);

	// ABX, ABY, IDY add one cycle if page boundary crossed
	return 1;
}

// M -> X; (NZ); Load X with memory
int CPU::LDX()
{
	x = *operand;

	checkNegative(x);
	checkZero(x);

	// ABY adds one cycle if page boundary crossed
	return 1;
}

// M -> Y; (NZ); Load Y with memory
int CPU::LDY()
{
	y = *operand;

	checkNegative(y);
	checkZero(y);

	// ABX adds one cycle if page boundary crossed
	return 1;
}

// M >> 1 -> M; (NZC); Left shift operand
int CPU::LSR()
{
	*operand >>= 1;

	clearFlag(Flag::Negative);
	checkZero(*operand);
	checkCarry(*operand);

	return 0;
}

// ; (); No operation
int CPU::NOP()
{
	return 0;
}

// A | M -> A; (NZ); OR memory and accumulator
int CPU::ORA()
{
	a |= *operand;

	// Check flags
	checkZero(a);
	checkNegative(a);

	// ABX, ABY, IDY addressing modes add one cycle if page boundary crossed
	return 1;
}

// Push A; (); Push accumulator on stack
int CPU::PHA()
{
	sp -= 1;
	memory->set(sp, a);

	return 0;
}

// Push P; (UB); Push processor status on stack
int CPU::PHP()
{
	uint8_t originalP = p;

	setFlag(Flag::Break);
	setFlag(Flag::Unused);

	sp -= 1;
	memory->set(sp, p);

	p = originalP;

	return 0;
}

// Pull A; (NZ); Pull acumulator from stack
int CPU::PLA()
{
	a = memory->read(sp);
	sp++;

	checkZero(a);
	checkNegative(a);

	return 0;
}

// Pull P; (); Pull processor status from stack
int CPU::PLP()
{
	// Bits 4 and 5 are kept same from prior to pulling status from stack
	uint8_t bit4 = hasFlag(Flag::Break);
	uint8_t bit5 = hasFlag(Flag::Unused);

	// Read new status from stack
	p = memory->read(sp);
	sp++;

	// Apply old bits to 4 and 5
	p ^= (-bit4 ^ p) & (1UL << 4);
	p ^= (-bit5 ^ p) & (1UL << 5);

	return 0;
}

// M << 1 <- C -> M; (NZC); Rotate one bit left with carry from right
int CPU::ROL()
{
	int16_t signedOperand = (*operand << 1) + hasFlag(Flag::Carry);
	*operand = *operand << 1 + hasFlag(Flag::Carry);

	checkNegative(*operand);
	checkZero(*operand);
	checkCarry(signedOperand);

	return 0;
}

// C -> M >> 1 -> M; (NZC); Rotate one bit right with carry from left
int CPU::ROR()
{
	int16_t signedOperand = (*operand >> 1) | (hasFlag(Flag::Carry) << 7);
	*operand = (*operand >> 1) | (hasFlag(Flag::Carry) << 7);

	checkNegative(*operand);
	checkZero(*operand);
	checkCarry(signedOperand);

	return 0;
}

// Pull P, Pull PC; (); Return from interrupt
int CPU::RTI()
{
	// TODO: Ignore bits 5 & 4

	p = memory->read(sp);
	sp++;

	pc = memory->read(sp);
	sp++;

	return 0;
}

// Pull PC, PC + 1 -> PC; (); Return from subroutine
int CPU::RTS()
{
	uint16_t returnAddress = memory->read(sp + 1) << 8;
	returnAddress |= memory->read(sp);

	pc = returnAddress - instructionLength;
	sp += 2;

	return 0;
}

// A - M - C -> A; (NZCV); Subtract memory from accumulator with borrow
int CPU::SBC()
{
	// To perform subtraction, can invert all bits of operand (giving -operand - 1) and pass to ADC
	uint8_t normalOperand = *operand;
	*operand = ~(*operand);
	
	int ret = ADC();
	*operand = normalOperand;

	return ret;
}

// 1 -> C; (C); Set carry flag
int CPU::SEC()
{
	setFlag(Flag::Carry);
	return 0;
}

// 1 -> D; (D); Set decimal flag
int CPU::SED()
{
	setFlag(Flag::Decimal);
	return 0;
}

// 1 -> I; (I); Set interrupt flag
int CPU::SEI()
{
	setFlag(Flag::Interrupt);
	return 0;
}

// A -> M; (); Store accumulator in memory
int CPU::STA()
{
	*operand = a;
	return 0;
}

// X -> M; (); Store X in memory
int CPU::STX()
{
	*operand = x;
	return 0;
}

// Y -> M; (); Store Y in memory
int CPU::STY()
{
	*operand = y;
	return 0;
}

// A -> X; (NZ); Transfer accumulator to X
int CPU::TAX()
{
	x = a;

	// Check flags
	checkNegative(x);
	checkZero(x);

	return 0;
}

// A -> Y; (NZ); Transfer accumulator to Y
int CPU::TAY()
{
	y = a;

	// Check flags
	checkNegative(y);
	checkZero(y);

	return 0;
}

// SP -> X; (NZ); Transfer stack pointer to X
int CPU::TSX()
{
	x = sp;

	// Check flags
	checkNegative(x);
	checkZero(x);

	return 0;
}

// X -> A; (NZ); Transfer X to accumulator
int CPU::TXA()
{
	a = x;

	// Check flags
	checkNegative(a);
	checkZero(a);

	return 0;
}

// X -> SP; (); Transfer X to stack pointer
int CPU::TXS()
{
	sp = x;
	return 0;
}

// Y -> A; (NZ); Transfer Y to accumulator
int CPU::TYA()
{
	a = y;

	// Check flags
	checkNegative(a);
	checkZero(a);

	return 0;
}
