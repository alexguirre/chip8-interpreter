#pragma once
#include <array>
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
	constexpr std::array<std::uint8_t, FontsetTotalByteSize> Fontset
	{
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

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