#pragma once
#include <array>
#include <cstdint>
#include <filesystem>
#include <chrono>
#include <optional>
#include <functional>
#include "Instructions.h"
#include "Display.h"
#include "Keyboard.h"
#include "Sound.h"

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
	std::array<bool, CKeyboard::NumberOfKeys> Keyboard;

	SContext();
	
	void Reset();

	inline std::uint8_t X() const { return (IR & 0x0F00) >> 8; }
	inline std::uint8_t Y() const { return (IR & 0x00F0) >> 4; }
	inline std::uint16_t NNN() const { return (IR & 0x0FFF); }
	inline std::uint8_t KK() const { return (IR & 0x00FF); }
	inline std::uint8_t N() const { return (IR & 0x000F); }
};

class CInterpreter
{
public:
	using Clock = std::chrono::high_resolution_clock;

	static constexpr std::uint16_t ProgramStartAddress{ 0x200 };
	static constexpr std::size_t InstructionByteSize{ 2 };	// Size of an instruction in bytes
	static constexpr std::size_t FontsetCharByteSize{ 5 };
	static const std::array<std::uint8_t, 16 * FontsetCharByteSize> Fontset;
	
	static constexpr std::size_t CyclesHz{ 600 };	// Number of instructions executed per second
	static constexpr std::chrono::milliseconds CyclesRateMs{ static_cast<std::size_t>((1.0 / CyclesHz) * 1000.0 + 0.5) };
	static constexpr std::size_t TimersHz{ 60 };	// Number of times the timers are decreased per second
	static constexpr std::chrono::milliseconds TimersRateMs{ static_cast<std::size_t>((1.0 / TimersHz) * 1000.0 + 0.5) };
	
	static constexpr double BeepFrequency{ 550.0 };
	static constexpr std::chrono::milliseconds BeepDuration{ 50 };

private:
	SContext mContext;
	std::unique_ptr<CDisplay> mDisplay;
	std::unique_ptr<CKeyboard> mKeyboard;
	std::unique_ptr<CSound> mSound;
	Clock::time_point mLastCycleTime;
	Clock::time_point mLastTimerTickTime;
	bool mPaused;

public:
	CInterpreter();

	CInterpreter(CInterpreter&&) = default;
	CInterpreter& operator=(CInterpreter&&) = default;

	CInterpreter(const CInterpreter&) = delete;
	CInterpreter& operator=(const CInterpreter&) = delete;

	inline const SContext& Context() const { return mContext; }
	inline const CDisplay& Display() const { return *mDisplay; }
	inline const CKeyboard& Keyboard() const { return *mKeyboard; }
	inline const CSound& Sound() const { return *mSound; }
	inline bool IsPaused() const { return mPaused; }

	void Pause(bool pause);
	void Update();
	void Step();
	void RenderDisplay();

	void LoadProgram(const std::filesystem::path& filePath);
	void LoadState(const std::filesystem::path& filePath);
	void SaveState(const std::filesystem::path& filePath) const;
	const SInstruction& FindInstruction(std::uint16_t opcode) const;
	std::optional<std::reference_wrapper<const SInstruction>> TryFindInstruction(std::uint16_t opcode) const;

private:
	void DoCycle();
	void DoTimerTick();
	void DoBeep();
};

