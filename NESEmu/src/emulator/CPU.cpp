#include "CPU.h"

#include <stdio.h>
#include <iomanip>
#include <memory>
#include <sstream>

// TODO: Handle interrupts (IRQ, NMI)
// TODO: Use modern C++ constructs (new-style casts, smart pointers, etc)

// Debugging
static bool constexpr DEBUG_LOG = true;
static char debugBuf[100];

// Address of the low byte of various jump vectors
static constexpr uint16_t NMI_VECTOR = 0xFFFA;
static constexpr uint16_t RES_VECTOR = 0xFFFC;
static constexpr uint16_t IRQ_VECTOR = 0xFFFE;

// Convenience macros for defining CPU instructions
#define _I(NAME, RUN, MODE, CYCLES) { NAME, &CPU::RUN, &CPU::MODE, CYCLES }
#define _XXX(MODE, CYCLES) { "XXX", &CPU::XXX, &CPU::MODE, CYCLES }
#define _NOP(MODE, CYCLES) { "NOP*", &CPU::NOP, &CPU::MODE, CYCLES }

// Since the SP is only 8-bit, and the stack starts at 0x0100, this is a utility to calculate the full SP address
#define SP_ADDRESS (sp + 0x0100)

// Initialize CPU
CPU::CPU(Memory &memory) : logger("..\\logs\\cpu.log"), memory(memory)
{
	a = 0x00;
	x = 0x00;
	y = 0x00;
	p = 0x24;
	pc = 0x00;
	sp = 0xFD;
	opcode = 0x00;
	totalCycles = 0;
	cycles = 0;
	operand = { OperandType::Invalid, 0x0000 };

	// Initialize instruction table
	// Hi-nibble on vertical, Lo-nibble on horizontal
	instructions =
	{
		//        -0                      -1                 -2                      -3            -4                             -5                      -6                 -7                 -8                      -9                        -A                 -B            -C                             -D                      -E                 -F                                                                 
		_I("BRK", BRK, IMP, 7), _I("ORA", ORA, IDX, 6), _XXX(IMM, 2),			_XXX(ZPX, 3), _NOP(ZPG, 3),				_I("ORA", ORA, ZPG, 3), _I("ASL", ASL, ZPG, 5), _XXX(ZPG, 5), _I("PHP", PHP, IMP, 3), _I("ORA", ORA, IMM, 2),	_I("ASL", ASL, ACC, 2), _XXX(IMM, 2), _NOP(ABS, 4),				_I("ORA", ORA, ABS, 4), _I("ASL", ASL, ABS, 6), _XXX(ABS, 6),
		_I("BPL", BPL, REL, 2), _I("ORA", ORA, IDY, 5), _XXX(IMM, 2),			_XXX(ZPY, 3), _NOP(ZPX, 4),				_I("ORA", ORA, ZPX, 4), _I("ASL", ASL, ZPX, 6), _XXX(ZPX, 6), _I("CLC", CLC, IMP, 2), _I("ORA", ORA, ABY, 4),	_NOP(IMP, 2),			_XXX(ABY, 7), _NOP(ABX, 4),				_I("ORA", ORA, ABX, 4), _I("ASL", ASL, ABX, 7), _XXX(ABX, 7),
		_I("JSR", JSR, ABS, 6), _I("AND", AND, IDX, 6), _XXX(IMM, 2),			_XXX(ZPX, 3), _I("BIT", BIT, ZPG, 3),	_I("AND", AND, ZPG, 3), _I("ROL", ROL, ZPG, 5), _XXX(ZPG, 5), _I("PLP", PLP, IMP, 4), _I("AND", AND, IMM, 2),	_I("ROL", ROL, ACC, 2), _XXX(IMM, 2), _I("BIT", BIT, ABS, 4),	_I("AND", AND, ABS, 4), _I("ROL", ROL, ABS, 6), _XXX(ABS, 6),
		_I("BMI", BMI, REL, 2), _I("AND", AND, IDY, 5), _XXX(IMM, 2),			_XXX(ZPY, 3), _NOP(ZPX, 4),				_I("AND", AND, ZPX, 4), _I("ROL", ROL, ZPX, 6), _XXX(ZPX, 6), _I("SEC", SEC, IMP, 2), _I("AND", AND, ABY, 4),	_NOP(IMP, 2),			_XXX(ABY, 7), _NOP(ABX, 4),				_I("AND", AND, ABX, 4), _I("ROL", ROL, ABX, 7), _XXX(ABX, 7),
		_I("RTI", RTI, IMP, 6), _I("EOR", EOR, IDX, 6), _XXX(IMM, 2),			_XXX(ZPX, 3), _NOP(ZPG, 3),				_I("EOR", EOR, ZPG, 3), _I("LSR", LSR, ZPG, 5), _XXX(ZPG, 5), _I("PHA", PHA, IMP, 3), _I("EOR", EOR, IMM, 2),	_I("LSR", LSR, ACC, 2), _XXX(IMM, 2), _I("JMP", JMP, ABS, 3),	_I("EOR", EOR, ABS, 4), _I("LSR", LSR, ABS, 6), _XXX(ABS, 6),
		_I("BVC", BVC, REL, 2), _I("EOR", EOR, IDY, 5), _XXX(IMM, 2),			_XXX(ZPY, 3), _NOP(ZPX, 4),				_I("EOR", EOR, ZPX, 4), _I("LSR", LSR, ZPX, 6), _XXX(ZPX, 6), _I("CLI", CLI, IMP, 2), _I("EOR", EOR, ABY, 4),	_NOP(IMP, 2),			_XXX(ABY, 7), _NOP(ABX, 4),				_I("EOR", EOR, ABX, 4), _I("LSR", LSR, ABX, 7), _XXX(ABX, 7),
		_I("RTS", RTS, IMP, 6), _I("ADC", ADC, IDX, 6), _XXX(IMM, 2),			_XXX(ZPX, 3), _NOP(ZPG, 3),				_I("ADC", ADC, ZPG, 3), _I("ROR", ROR, ZPG, 5), _XXX(ZPG, 5), _I("PLA", PLA, IMP, 4), _I("ADC", ADC, IMM, 2),	_I("ROR", ROR, ACC, 2), _XXX(IMM, 2), _I("JMP", JMP, IND, 5),	_I("ADC", ADC, ABS, 4), _I("ROR", ROR, ABS, 4), _XXX(ABS, 6),
		_I("BVS", BVS, REL, 2), _I("ADC", ADC, IDY, 5), _XXX(IMM, 2),			_XXX(ZPY, 3), _NOP(ZPX, 3),				_I("ADC", ADC, ZPX, 4), _I("ROR", ROR, ZPX, 6), _XXX(ZPX, 6), _I("SEI", SEI, IMP, 2), _I("ADC", ADC, ABY, 4),	_NOP(IMP, 2),			_XXX(ABY, 7), _NOP(ABX, 4),				_I("ADC", ADC, ABX, 4), _I("ROR", ROR, ABX, 7), _XXX(ABX, 7),
		_NOP(IMM, 2),			_I("STA", STA, IDX, 6), _NOP(IMM, 2),			_XXX(ZPX, 3), _I("STY", STY, ZPG, 3),	_I("STA", STA, ZPG, 3), _I("STX", STX, ZPG, 3), _XXX(ZPG, 5), _I("DEY", DEY, IMP, 2), _NOP(IMM, 2),				_I("TXA", TXA, IMP, 2), _XXX(IMM, 2), _I("STY", STY, ABS, 4),	_I("STA", STA, ABS, 4), _I("STX", STX, ABS, 4), _XXX(ABS, 4),
		_I("BCC", BCC, REL, 2), _I("STA", STA, IDY, 6), _XXX(IMM, 2),			_XXX(ZPY, 3), _I("STY", STY, ZPX, 4),	_I("STA", STA, ZPX, 4), _I("STX", STX, ZPY, 4), _XXX(ZPX, 6), _I("TYA", TYA, IMP, 2), _I("STA", STA, ABY, 5),	_I("TXS", TXS, IMP, 2), _XXX(ABY, 5), _XXX(ABX, 5),				_I("STA", STA, ABX, 5), _XXX(ABY, 5),			_XXX(ABY, 5),
		_I("LDY", LDY, IMM, 2), _I("LDA", LDA, IDX, 6), _I("LDX", LDX, IMM, 2), _XXX(ZPX, 3), _I("LDY", LDY, ZPG, 3),	_I("LDA", LDA, ZPG, 3), _I("LDX", LDX, ZPG, 3), _XXX(ZPG, 5), _I("TAY", TAY, IMP, 2), _I("LDA", LDA, IMM, 2),	_I("TAX", TAX, IMP, 2), _XXX(IMM, 2), _I("LDY", LDY, ABS, 4),	_I("LDA", LDA, ABS, 4), _I("LDX", LDX, ABS, 4), _XXX(ABS, 4),
		_I("BCS", BCS, REL, 2), _I("LDA", LDA, IDY, 5), _XXX(IMM, 2),			_XXX(ZPY, 3), _I("LDY", LDY, ZPX, 4),	_I("LDA", LDA, ZPX, 4), _I("LDX", LDX, ZPY, 4), _XXX(ZPX, 6), _I("CLV", CLV, IMP, 2), _I("LDA", LDA, ABY, 4),	_I("TSX", TSX, IMP, 2), _XXX(ABY, 7), _I("LDY", LDY, ABX, 4),	_I("LDA", LDA, ABX, 4), _I("LDX", LDX, ABY, 4), _XXX(ABY, 4),
		_I("CPY", CPY, IMM, 2), _I("CMP", CMP, IDX, 6), _NOP(IMM, 2),			_XXX(ZPX, 3), _I("CPY", CPY, ZPG, 3),	_I("CMP", CMP, ZPG, 3), _I("DEC", DEC, ZPG, 5), _XXX(ZPG, 5), _I("INY", INY, IMP, 2), _I("CMP", CMP, IMM, 2),	_I("DEX", DEX, IMP, 2), _XXX(IMM, 2), _I("CPY", CPY, ABS, 4),	_I("CMP", CMP, ABS, 4), _I("DEC", DEC, ABS, 6), _XXX(ABS, 6),
		_I("BNE", BNE, REL, 2), _I("CMP", CMP, IDY, 5), _XXX(IMM, 2),			_XXX(ZPY, 3), _NOP(ZPX, 4),				_I("CMP", CMP, ZPX, 4), _I("DEC", DEC, ZPX, 6), _XXX(ZPX, 6), _I("CLD", CLD, IMP, 2), _I("CMP", CMP, ABY, 4),	_NOP(IMP, 2),			_XXX(ABY, 7), _NOP(ABX, 4),				_I("CMP", CMP, ABX, 4), _I("DEC", DEC, ABX, 7), _XXX(ABX, 7),
		_I("CPX", CPX, IMM, 2), _I("SBC", SBC, IDX, 6), _NOP(IMM, 2),			_XXX(ZPX, 3), _I("CPX", CPX, ZPG, 3),	_I("SBC", SBC, ZPG, 3), _I("INC", INC, ZPG, 5), _XXX(ZPG, 5), _I("INX", INX, IMP, 2), _I("SBC", SBC, IMM, 2),	_I("NOP", NOP, IMP, 2), _XXX(IMM, 2), _I("CPX", CPX, ABS, 4),	_I("SBC", SBC, ABS, 4), _I("INC", INC, ABS, 6), _XXX(ABS, 6),
		_I("BEQ", BEQ, REL, 2), _I("SBC", SBC, IDY, 5), _XXX(IMM, 2),			_XXX(ZPY, 3), _NOP(ZPX, 4),				_I("SBC", SBC, ZPX, 4), _I("INC", INC, ZPX, 6), _XXX(ZPX, 6), _I("SED", SED, IMP, 2), _I("SBC", SBC, ABY, 4),	_NOP(IMP, 2),			_XXX(ABY, 7), _NOP(ABS, 4),				_I("SBC", SBC, ABX, 4), _I("INC", INC, ABX, 7), _XXX(ABX, 7)
	};
}

void CPU::reset()
{
	// Initialize other registers
	a = 0x00;
	x = 0x00;
	y = 0x00;
	p = 0x24;
	sp = 0xFD;
	opcode = 0x00;
	totalCycles = 0;
	cycles = 0;
	operand = { OperandType::Invalid, 0x0000 };

	// Find reset vector at 0xFFFC, and set initial PC
	jumpVector(RES_VECTOR);

	printf("CPU reset\n");
	printf("\tSet PC to $%X\n", pc);

	// Reset takes 8 cycles
	cycles += 8;
	totalCycles += 8;
}

void CPU::step()
{
	if (cycles > 0)
	{
		cycles--;
	}
	else
	{
		// Poll for OAM data transfer request
		if (memory.pollOamTransfer())
		{
			memory.dispatchOamTransfer();

			// OAM data transfer takes 513 (+1) cycles
			cycles = 513;

			// +1 cycle if on odd CPU cycle
			if (totalCycles % 2 != 0)
			{
				cycles++;
			}

			return;
		}

		// Poll for NMI interrupts
		if (memory.pollNmi())
		{
			// TODO: Make interrupt func
			uint16_t returnAddress = pc;
			memory.write(SP_ADDRESS, returnAddress >> 8); // Return address high byte
			memory.write(SP_ADDRESS - 1, returnAddress & 0x00FF); // Return address low byte
			memory.write(SP_ADDRESS - 2, p); // Status

			setFlag(Flag::Interrupt);

			// Set SP to next empty slot
			sp -= 3;

			jumpVector(NMI_VECTOR);
		}

		// Process opcode
		opcode = memory.read(pc);
		Instruction ins = instructions[opcode];
		instructionLength = 1;
		int modeExtra = (this->*ins.addressingMode)();

		// Write debug info to log
		if (DEBUG_LOG)
		{
			char opcodeBuf[11];

			switch (instructionLength)
			{
			case 1:
				snprintf(opcodeBuf, 9, "%02X      ", opcode);
				break;
			case 2:
				snprintf(opcodeBuf, 9, "%02X %02X    ", opcode, memory.read(pc + 1, true));
				break;
			case 3:
				snprintf(opcodeBuf, 9, "%02X %02X %02X", opcode, memory.read(pc + 1, true), memory.read(pc + 2, true));
				break;
			}

			snprintf(debugBuf, 100, "%04X  %s  %s\t\tA:%02X X:%02X Y:%02X P:%02X SP:%02X\tCYC:%d\n", pc, opcodeBuf, ins.instruction.c_str(), a, x, y, p, sp, totalCycles);
			logger.write(debugBuf);
		}

		int runExtra = (this->*ins.run)();

		// TODO: Fix cycle calculation, runs behind
		// Likely due to not adding extra cycles on page boundary being crossed
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

void CPU::setStatusBit(uint8_t bit, uint8_t value)
{
	p ^= (-value ^ p) & (1UL << bit);
}

void CPU::setFlagValue(Flag flag, uint8_t value)
{
	setStatusBit((uint8_t)flag, value);
}

bool CPU::hasFlag(Flag flag) const
{
	return (p & (uint8_t)flag) > 0;
}

void CPU::setPC(uint16_t pc)
{
	this->pc = pc;
}

CPU::State CPU::getState() const
{
	State state =
	{
		state.a = a,
		state.x = x,
		state.y = y,
		state.p = p,
		state.sp = sp,
		state.pc = pc,
		state.totalCycles = totalCycles,
		state.instructionLength = instructionLength,

		state.opcode = opcode,
		state.cycles = totalCycles
	};

	return state;
}

void CPU::writeOperand(uint8_t value, bool skipCallback)
{
	switch (operand.type)
	{
	case OperandType::Invalid:
		printf("Tried writing to an invalid operand\n");
		break;
	case OperandType::Address:
		memory.write(operand.address, value);
		break;
	case OperandType::Accumulator:
		a = value;
		break;
	}
}

uint8_t CPU::readOperand(bool skipCallback)
{
	switch (operand.type)
	{
	case OperandType::Invalid:
		printf("Tried reading from an invalid operand\n");
		return 0;
	case OperandType::Address:
		return memory.read(operand.address);
	case OperandType::Accumulator:
		return a;
	}

	// This should never happen
	return 0;
}

void CPU::checkOverflow(int8_t target, int8_t result)
{
	int8_t signedOperand = readOperand();

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
	int8_t offset = readOperand();
	pc += offset;

	// TODO: Check if branch occurs on same page or different page
	return 1;
}

void CPU::jumpVector(uint16_t address)
{
	// Move PC to address from memory (little endian)
	pc = (memory.read(address + 1) << 8) | memory.read(address);
}

/* Addressing Modes */

// Implicit or implied
int CPU::IMP()
{
	// No operand
	operand.type = OperandType::Invalid;
	instructionLength = 1;

	return 0;
}

// Accumulator
int CPU::ACC()
{
	// Operand is accumulator
	operand.type = OperandType::Accumulator;
	instructionLength = 1;

	return 0;
}

// Immediate
int CPU::IMM()
{
	// Operand given in 1 byte after instruction
	operand = { OperandType::Address,  pc + 1u };
	instructionLength = 2;

	return 0;
}

// Zero-page
int CPU::ZPG()
{
	// Operand is a memory address in range $0000-$00FF, in 1 byte after instruction
	uint16_t address = memory.read(pc + 1);
	operand = { OperandType::Address, address };
	instructionLength = 2;

	return 0;
}

// Zero-page, X
int CPU::ZPX()
{
	// Operand is a memory address in range $0000-$00FF added to X register
	uint16_t address = memory.read(pc + 1) + x;

	// Take zero page wrap-around into account (wrap if past $00FF)
	address %= 0x0100;

	operand = { OperandType::Address, address };
	instructionLength = 2;

	return 0;
}

// Zero-page, Y
int CPU::ZPY()
{
	// Operand is a memory address in range $0000-$00FF added to Y register
	uint16_t address = memory.read(pc + 1) + y;

	// Take zero page wrap-around into account (wrap if past $00FF)
	address %= 0x0100;

	operand = { OperandType::Address, address };
	instructionLength = 2;

	return 0;
}

// Relative (Only used by branch) [+]
int CPU::REL()
{
	// Operand is a SIGNED bit after the instruction
	operand = { OperandType::Address, pc + 1u };
	instructionLength = 2;

	return 1;
}

// Absolute
int CPU::ABS()
{
	// Operand is an address after the instruction, stored in little-endian
	uint16_t address = memory.read(pc + 2) << 8;
	address |= memory.read(pc + 1);
	operand = { OperandType::Address, address };

	// Jump target is the 16-bit address after the instruction
	jumpTarget = address;
	instructionLength = 3;

	return 0;
}

// Absolute, X [+]
int CPU::ABX()
{
	// Operand is an address after the instruction, stored in little-endian, added to X
	uint16_t address = memory.read(pc + 2) << 8;
	address |= memory.read(pc + 1);
	operand = { OperandType::Address, (uint16_t)(address + x) };
	instructionLength = 3;

	// TODO: Deal with carry
	// TODO: Check if violating page boundary
	return 1;
}

// Absolute, Y [+]
int CPU::ABY()
{
	// Operand is an address after the instruction, stored in little-endian, added to Y
	uint16_t address = memory.read(pc + 2) << 8;
	address |= memory.read(pc + 1);
	operand = { OperandType::Address, (uint16_t)(address + y) };
	instructionLength = 3;

	// TODO: Deal with carry
	// TODO: Check if violating page boundary
	return 1;
}

// Indirect (only used by JMP)
int CPU::IND()
{
	// Jump target is a 16-bit value at address specified after the instruction, stored in little-endian
	uint16_t address = memory.read(pc + 2) << 8;
	address |= memory.read(pc + 1);

	// Check if LSB falls on page boundary
	if ((address & 0x00FF) == 0x00FF)
	{
		// If fetching LSB from page boundary, then wrap around and fetch MSB from page start
		jumpTarget = memory.read(address & 0xFF00) << 8;
		jumpTarget |= memory.read(address);
	}
	else
	{
		// If LSB is not on page boundary, fetch as normal
		jumpTarget = memory.read(address + 1) << 8;
		jumpTarget |= memory.read(address);
	}

	// Operand unused
	operand.type = OperandType::Invalid;
	instructionLength = 3;

	return 0;
}

// Indirect, X
int CPU::IDX()
{
	// Read the pointer address after the instruction and add X register to it
	uint8_t pointer = memory.read(pc + 1);
	pointer += x;

	// Read the address located at the pointer (high-byte first)
	// Also ensuring that (pointer + 1) for fetching high byte follows zero-page wrap-around
	uint16_t address = memory.read((pointer + 1) & 0xFF) << 8;
	address |= memory.read(pointer);

	operand = { OperandType::Address, address };
	instructionLength = 2;

	return 0;
}

// Indirect, Y [+]
int CPU::IDY()
{
	// Read the pointer address after the instruction
	uint8_t pointer = memory.read(pc + 1);

	// Read the address located at the pointer (high-byte first), and add register Y to it
	// Also ensuring that (pointer + 1) for fetching high byte follows zero-page wrap-around
	uint16_t address = memory.read((pointer + 1) & 0xFF) << 8;
	address |= memory.read(pointer);
	address += y;

	operand = { OperandType::Address, address };
	instructionLength = 2;

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
	uint16_t result = a + readOperand() + hasFlag(Flag::Carry);

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
	a &= readOperand();

	checkNegative(a);
	checkZero(a);

	// ABX, ABY, IDY addressing modes add one cycle if page boundary crossed
	return 1;
}

// M << 1 -> M; (NZC); Shift left by one bit
int CPU::ASL()
{
	uint8_t operandVal = readOperand();

	// Shift original bit 7 into carry
	if (!!(operandVal & 0x80) == 0)
	{
		clearFlag(Flag::Carry);
	}
	else
	{
		setFlag(Flag::Carry);
	}

	// Shift operand left one bit
	operandVal <<= 1;

	checkNegative(operandVal);
	checkZero(operandVal);

	writeOperand(operandVal);

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
	uint8_t operandVal = readOperand();

	// Bits 7 and 6 transferred to the status register
	uint8_t bit7 = !!(operandVal >> 7);
	setStatusBit(7, bit7);

	uint8_t bit6 = !!((operandVal >> 6) & 0b01);
	setStatusBit(6, bit6);

	// Operand and accumulator ANDed to get zero flag value
	if ((a & operandVal) == 0)
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
	uint16_t returnAddress = pc + 2;
	memory.write(SP_ADDRESS, returnAddress >> 8); // Return address high byte
	memory.write(SP_ADDRESS - 1, returnAddress & 0x00FF); // Return address low byte
	memory.write(SP_ADDRESS - 2, p); // Status

	// TODO: Not sure if I flag needs to be set here?
	setFlag(Flag::Break);
	setFlag(Flag::Interrupt);

	// Set SP to next empty slot
	sp -= 3;

	// Move PC to IRQ vector
	jumpVector(IRQ_VECTOR);

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
	uint8_t operandVal = readOperand();
	uint8_t result = a - operandVal;

	checkNegative(result);
	checkZero(result);

	if (a >= operandVal)
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
	uint8_t operandVal = readOperand();
	uint8_t result = x - operandVal;

	checkNegative(result);
	checkZero(result);

	if (x >= operandVal)
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
	uint8_t operandVal = readOperand();
	uint8_t result = y - operandVal;

	checkNegative(result);
	checkZero(result);

	if (y >= operandVal)
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
	uint8_t operandVal = readOperand();
	operandVal--;

	checkNegative(operandVal);
	checkZero(operandVal);

	writeOperand(operandVal);

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
	a = a ^ readOperand();

	checkNegative(a);
	checkZero(a);

	// ABX, ABY, IDY require extra cycle if page boundary crossed
	return 1;
}

// M + 1 -> M; (NZ); Increment memory by one
int CPU::INC()
{
	uint8_t operandVal = readOperand();
	operandVal++;

	checkNegative(operandVal);
	checkZero(operandVal);
	
	writeOperand(operandVal);

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
	uint16_t returnAddress = pc + 2;
	memory.write(SP_ADDRESS, (returnAddress & 0xFF00) >> 8);
	memory.write(SP_ADDRESS - 1, returnAddress & 0x00FF);

	sp -= 2;
	pc = jumpTarget - instructionLength;

	return 0;
}

// M -> A; (NZ); Load accumulator with memory
int CPU::LDA()
{
	a = readOperand();

	checkNegative(a);
	checkZero(a);

	// ABX, ABY, IDY add one cycle if page boundary crossed
	return 1;
}

// M -> X; (NZ); Load X with memory
int CPU::LDX()
{
	x = readOperand();

	checkNegative(x);
	checkZero(x);

	// ABY adds one cycle if page boundary crossed
	return 1;
}

// M -> Y; (NZ); Load Y with memory
int CPU::LDY()
{
	y = readOperand();

	checkNegative(y);
	checkZero(y);

	// ABX adds one cycle if page boundary crossed
	return 1;
}

// M >> 1 -> M; (NZC); Logically right shift operand
int CPU::LSR()
{
	uint8_t operandVal = readOperand();

	// Original bit 0 shifted into carry
	// setFlagValue(Flag::Carry, !!(*operand & 0x01));
	if (!!(operandVal & 0x01) == 0)
	{
		clearFlag(Flag::Carry);
	}
	else
	{
		setFlag(Flag::Carry);
	}

	// Shift operand to right 1 bit
	operandVal >>= 1;

	clearFlag(Flag::Negative);
	checkZero(operandVal);

	writeOperand(operandVal);

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
	a |= readOperand();

	// Check flags
	checkZero(a);
	checkNegative(a);

	// ABX, ABY, IDY addressing modes add one cycle if page boundary crossed
	return 1;
}

// Push A; (); Push accumulator on stack
int CPU::PHA()
{
	memory.write(SP_ADDRESS, a);
	sp--;

	return 0;
}

// Push P; (UB); Push processor status on stack
int CPU::PHP()
{
	uint8_t originalP = p;

	setFlag(Flag::Break);
	setFlag(Flag::Unused);

	memory.write(SP_ADDRESS, p);
	sp--;

	p = originalP;

	return 0;
}

// Pull A; (NZ); Pull acumulator from stack
int CPU::PLA()
{
	sp++;
	a = memory.read(SP_ADDRESS);

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
	sp++;
	p = memory.read(SP_ADDRESS);

	// Apply old bits to 4 and 5
	setStatusBit(4, bit4);
	setStatusBit(5, bit5);

	return 0;
}

// M << 1 <- C -> M; (NZC); Rotate one bit left with carry from right
int CPU::ROL()
{
	uint8_t operandVal = readOperand();

	// Get original bit 7 for new carry
	uint8_t bit7 = !!(operandVal & 0x80);
	operandVal <<= 1;

	// Set old carry to bit 1
	operandVal |= hasFlag(Flag::Carry);

	// Set new carry to original bit 7
	if (bit7 == 0)
	{
		clearFlag(Flag::Carry);
	}
	else
	{
		setFlag(Flag::Carry);
	}

	// Other flag checks
	checkNegative(operandVal);
	checkZero(operandVal);

	writeOperand(operandVal);

	return 0;
}

// C -> M >> 1 -> M; (NZC); Rotate one bit right with carry from left
int CPU::ROR()
{
	uint8_t operandVal = readOperand();

	// Get original bit 0 for new carry
	uint8_t bit0 = !!(operandVal & 0x01);
	operandVal >>= 1;

	// Set old carry to bit 7
	operandVal |= (hasFlag(Flag::Carry) << 7);

	// Set new carry to original bit 0
	if (bit0 == 0)
	{
		clearFlag(Flag::Carry);
	}
	else
	{
		setFlag(Flag::Carry);
	}

	// Other flag checks
	checkNegative(operandVal);
	checkZero(operandVal);

	writeOperand(operandVal);

	return 0;
}

// Pull P, Pull PC; (); Return from interrupt
int CPU::RTI()
{
	// Bits 4 and 5 are kept same from prior to pulling status from stack
	uint8_t bit4 = hasFlag(Flag::Break);
	uint8_t bit5 = hasFlag(Flag::Unused);

	// Read new status from stack
	sp++;
	p = memory.read(SP_ADDRESS);

	// Apply old bits to 4 and 5
	setStatusBit(4, bit4);
	setStatusBit(5, bit5);

	// Read PC from stack (in high, low order)
	uint16_t returnAddress = memory.read(SP_ADDRESS + 2) << 8;
	returnAddress |= memory.read(SP_ADDRESS + 1);

	// PC should be set to return address pulled from stack
	// But PC will be automatically incremented later, so decrement now to negate
	pc = returnAddress - 1;
	sp += 2;

	return 0;
}

// Pull PC, PC + 1 -> PC; (); Return from subroutine
int CPU::RTS()
{
	// Read PC from stack (in high, low order)
	uint16_t returnAddress = memory.read(SP_ADDRESS + 2) << 8;
	returnAddress |= memory.read(SP_ADDRESS + 1);

	// PC should be set to returnAddress + 1, but is automatically incremented later
	// Set PC to just return address to negate
	pc = returnAddress;
	sp += 2;

	return 0;
}

// A - M - C -> A; (NZCV); Subtract memory from accumulator with borrow
int CPU::SBC()
{
	// To perform subtraction, can invert all bits of operand (giving -operand - 1) and pass to ADC
	uint8_t originalOperand = readOperand(true);

	writeOperand(~originalOperand, true);
	int ret = ADC();
	writeOperand(originalOperand, true);

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
	writeOperand(a);
	return 0;
}

// X -> M; (); Store X in memory
int CPU::STX()
{
	writeOperand(x);
	return 0;
}

// Y -> M; (); Store Y in memory
int CPU::STY()
{
	writeOperand(y);
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
