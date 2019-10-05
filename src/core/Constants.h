#pragma once
#include <cstdlib>
#include <cstdint>
#include <chrono>

namespace c8::constants
{
	constexpr std::size_t NumberOfRegisters{ 16 };
	constexpr std::size_t StackSize{ 16 };
	constexpr std::size_t MemorySize{ 4096 };

	constexpr std::uint16_t ProgramStartAddress{ 0x200 };
	constexpr std::size_t InstructionByteSize{ 2 };	// Size of an instruction in bytes

	constexpr std::size_t FontsetCharByteSize{ 5 };
	constexpr std::size_t FontsetCharCount{ 16 };
	constexpr std::size_t FontsetTotalByteSize{ FontsetCharCount * FontsetCharByteSize };

	constexpr std::size_t CyclesHz{ 600 };	// Number of instructions executed per second
	constexpr std::chrono::milliseconds CyclesRate{ static_cast<std::size_t>((1.0 / CyclesHz) * 1000.0 + 0.5) };
	constexpr std::size_t TimersHz{ 60 };	// Number of times the timers are decreased per second
	constexpr std::chrono::milliseconds TimersRate{ static_cast<std::size_t>((1.0 / TimersHz) * 1000.0 + 0.5) };

	constexpr double BeepFrequency{ 550.0 };
	constexpr std::chrono::milliseconds BeepDuration{ 50 };

	constexpr std::size_t KeyboardKeyCount{ 16 };

	constexpr std::size_t DisplayResolutionWidth{ 64 };
	constexpr std::size_t DisplayResolutionHeight{ 32 };
}