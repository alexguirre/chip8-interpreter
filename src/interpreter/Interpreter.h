#pragma once
#include <array>
#include <cstdint>
#include <filesystem>
#include "Display.h"

struct SContext
{
	static constexpr std::size_t NumRegisters = 16;
	static constexpr std::size_t StackSize = 16;
	static constexpr std::size_t MemorySize = 4096;

	std::array<std::uint8_t, NumRegisters> V;	// General purpose registers
	std::uint16_t I;	// The memory address register
	std::uint16_t PC;	// The program counter
	std::uint8_t SP;	// The stack pointer
	std::uint8_t DT;	// The delay timer
	std::uint8_t ST;	// The sound timer
	std::uint16_t IR;	// The current instruction opcode
	std::array<std::uint16_t, StackSize> Stack;
	std::array<std::uint8_t, MemorySize> Memory;
	CDisplay::PixelBuffer PixelBuffer;
	bool PixelBufferDirty;

	SContext();
	
	void Reset();

	inline std::uint8_t X() const { return (IR & 0x0F00) >> 12; }
	inline std::uint8_t Y() const { return (IR & 0x00F0) >> 8; }
	inline std::uint16_t NNN() const { return (IR & 0x0FFF); }
	inline std::uint8_t KK() const { return (IR & 0x00FF); }
	inline std::uint8_t N() const { return (IR & 0x000F); }
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
	static constexpr std::uint16_t ProgramStartAddress = 0x200;
	static constexpr std::size_t FontsetCharByteSize = 5;
	static const std::array<std::uint8_t, 16 * FontsetCharByteSize> Fontset;

private:
	SContext mContext;
	std::unique_ptr<CDisplay> mDisplay;

public:
	CInterpreter();

	CInterpreter(CInterpreter&&) = default;
	CInterpreter& operator=(CInterpreter&&) = default;

	CInterpreter(const CInterpreter&) = delete;
	CInterpreter& operator=(const CInterpreter&) = delete;

	inline const SContext& Context() const { return mContext; }
	inline const CDisplay& Display() const { return *mDisplay; }

	void Update();

	void LoadProgram(const std::filesystem::path& filePath);
	const SInstruction& FindInstruction(std::uint16_t opcode);

	static const std::vector<SInstruction> Instructions;

private:
	void DoCycle();
};

