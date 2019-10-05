#pragma once
#include <array>
#include <cstdint>
#include <filesystem>
#include <chrono>
#include <optional>
#include <functional>
#include "Instructions.h"
#include "Constants.h"

namespace c8
{
	struct SContext
	{
		std::array<std::uint8_t, constants::NumberOfRegisters> V;	// General purpose registers
		std::uint16_t I;	// The memory address register
		std::uint16_t PC;	// The program counter
		std::uint8_t SP;	// The stack pointer
		std::uint8_t DT;	// The delay timer
		std::uint8_t ST;	// The sound timer
		std::uint16_t IR;	// The current instruction opcode
		std::array<std::uint16_t, constants::StackSize> Stack;
		std::array<std::uint8_t, constants::MemorySize> Memory;
		std::array<std::uint8_t, (constants::DisplayResolutionWidth * constants::DisplayResolutionHeight)> PixelBuffer;
		bool PixelBufferDirty;
		std::array<bool, constants::KeyboardKeyCount> Keyboard;

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

		static const std::array<std::uint8_t, constants::FontsetTotalByteSize> Fontset;

	private:
		SContext mContext;
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
		inline bool IsPaused() const { return mPaused; }

		void Pause(bool pause);
		void Update();
		void Step();

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
}
