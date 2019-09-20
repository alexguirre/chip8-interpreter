#pragma once
#include <array>
#include <cstdint>
#include <filesystem>
#include "Memory.h"

struct SContext
{
	static constexpr std::size_t NumRegisters = 16;


	std::array<std::uint8_t, NumRegisters> V;	// General purpose registers
	std::uint16_t I;	// The memory address register
	std::uint16_t PC;	// The program counter
	std::uint8_t SP;	// The stack pointer
	std::uint8_t DT;	// The delay timer
	std::uint8_t ST;	// The sound timer

	SContext();
	
	void Reset();
};

using FInstructionHandler = void(*)(SContext& context);

struct SInstruction
{
	std::string Name;
	FInstructionHandler Handler;
	std::uint16_t Opcode;
	std::uint16_t OpcodeMask;
};

class CInterpreter
{
public:
	static constexpr memptr_t ProgramStartAddress = 0x200;
	static constexpr std::size_t MaxROMSize = CMemory::SizeInBytes - ProgramStartAddress;

private:
	SContext mContext;
	CMemory mMemory;

public:
	CInterpreter() = default;

	CInterpreter(CInterpreter&&) = default;
	CInterpreter& operator=(CInterpreter&&) = default;

	CInterpreter(const CInterpreter&) = delete;
	CInterpreter& operator=(const CInterpreter&) = delete;

	inline const SContext& Context() const { return mContext; }
	inline const CMemory& Memory() const { return mMemory; }

	void LoadProgram(const std::filesystem::path& filePath);
	const SInstruction& FindInstruction(std::uint16_t opcode);

	static const std::vector<SInstruction> Instructions;
};

