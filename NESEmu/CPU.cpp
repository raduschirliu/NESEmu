#include <stdio.h>
#include "CPU.h"

// Convenience macros for defining CPU instructions
#define _I(NAME, RUN, MODE, CYCLES) { NAME, &CPU::RUN, &CPU::MODE, CYCLES }
#define _XXX() { "XXX", &CPU::XXX, &CPU::IMP, 2 }

// Initialize CPU
CPU::CPU() : logger("logs/cpu.txt")
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
		printf("Stepping...\n");
		printf("PC: %X\n", pc);
		printf("CYCLE: %d\n\n", totalCycles);
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
		printf("Executing\n");
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
			uint8_t bit = (p >> 7 - i) & 0b00000001;
			printf("%d", bit);
		}

		printf("\n");

		printf("A: %d | $%X\n", a, a);
		printf("X: %d | $%X\n", x, x);
		printf("Y: %d | $%X\n", y, y);
		printf("CYCLE: %d\n", totalCycles);
		printf("------------------------------\n\n");
		// ---- Debug

		int runExtra = (this->*ins.run)();

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

void CPU::setMemory(Memory *memory)
{
	this->memory = memory;
}

void CPU::checkOverflow(uint8_t target, uint8_t result)
{
	if (target > 0 && *operand > 0 && result < 0)
	{
		setFlag(Flag::Overflow);
	}
	else if (target < 0 && *operand < 0 && result > 0)
	{
		setFlag(Flag::Overflow);
	}
	else
	{
		clearFlag(Flag::Overflow);
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

	return 0;
}

// Zero-page, X
int CPU::ZPX()
{
	// Operand is a memory address in range $0000-$00FF added to X register
	uint16_t address = memory->read(pc + 1) + x;
	operand = memory->get(address);
	instructionLength++;

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
	// TODO: document
	uint16_t address = memory->read(pc + 2) << 8;
	address |= memory->read(pc + 1);
	operand = memory->get(address);
	instructionLength += 2;

	return 0;
}

// Indirect, X
int CPU::IDX()
{
	return 0;
}

// Indirect, Y [+]
int CPU::IDY()
{
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
	uint8_t result = a + *operand + hasFlag(Flag::Carry);

	// Carry flag
	uint16_t carryCheck = a + *operand + hasFlag(Flag::Carry);
	if (carryCheck > 255 || carryCheck < -255)
	{
		setFlag(Flag::Carry);
	}

	// Other flags
	checkZero(a);
	checkOverflow(a, result);
	checkNegative(a);

	a = result;

	// ABX, ABY, IDY add one cycle if page boundary crossed
	return 1;
}

// A & M -> A (NZ); Logical AND on accumulator and memory
int CPU::AND()
{
	a &= *operand;

	if (a == 0)
	{
		setFlag(Flag::Zero);
	}

	if ((a & 0b10000000) != 0)
	{
		setFlag(Flag::Negative);
	}

	// ABX, ABY, IDY addressing modes add one cycle if page boundary crossed
	return 1;
}

// M << 1 -> M; (NZC); Shift left one bit
int CPU::ASL()
{
	*operand = *operand << 1;
	
	// Check flags
	checkNegative(*operand);
	checkZero(*operand);

	// TODO: Check carry flag

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

int CPU::BIT()
{
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

int CPU::BRK()
{
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

int CPU::CMP()
{
	return 0;
}

int CPU::CPX()
{
	return 0;
}

int CPU::CPY()
{
	return 0;
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

int CPU::EOR()
{
	return 0;
}

int CPU::INC()
{
	return 0;
}

int CPU::INX()
{
	return 0;
}

int CPU::INY()
{
	return 0;
}

int CPU::JMP()
{
	uint16_t address = (*(operand + 1) << 8) | *operand;
	pc = address - instructionLength;
	return 0;
}

int CPU::JSR()
{
	return 0;
}

int CPU::LDA()
{
	return 0;
}

int CPU::LDX()
{
	return 0;
}

int CPU::LDY()
{
	return 0;
}

int CPU::LSR()
{
	return 0;
}

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

int CPU::PHA()
{
	return 0;
}

int CPU::PHP()
{
	return 0;
}

int CPU::PLA()
{
	return 0;
}

int CPU::PLP()
{
	return 0;
}

int CPU::ROL()
{
	return 0;
}

int CPU::ROR()
{
	return 0;
}

int CPU::RTI()
{
	return 0;
}

int CPU::RTS()
{
	return 0;
}

int CPU::SBC()
{
	return 0;
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
