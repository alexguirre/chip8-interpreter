#pragma once
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>

namespace c8::constants
{
	constexpr std::size_t NumberOfRegisters{ 16 };
	constexpr std::size_t StackSize{ 16 };
	constexpr std::size_t MemorySize{ 4096 };

	constexpr std::uint16_t ProgramStartAddress{ 0x200 };
	constexpr std::size_t InstructionByteSize{ 2 }; // Size of an instruction in bytes

	constexpr std::uint16_t FontsetAddress{ 0x0 };
	constexpr std::size_t FontsetCharByteSize{ 5 };
	constexpr std::size_t FontsetCharCount{ 16 };
	constexpr std::size_t FontsetTotalByteSize{ FontsetCharCount * FontsetCharByteSize };
	constexpr std::array<std::uint8_t, FontsetTotalByteSize> Fontset{
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

	constexpr std::size_t CyclesHz{ 600 }; // Number of instructions executed per second
	constexpr std::chrono::milliseconds CyclesRate{ static_cast<std::size_t>(
		(1.0 / CyclesHz) * 1000.0 + 0.5) };
	constexpr std::size_t TimersHz{ 60 }; // Number of times the timers are decreased per second
	constexpr std::chrono::milliseconds TimersRate{ static_cast<std::size_t>(
		(1.0 / TimersHz) * 1000.0 + 0.5) };

	constexpr double BeepFrequency{ 550.0 };
	constexpr std::chrono::milliseconds BeepDuration{ 50 };

	constexpr std::size_t KeyboardKeyCount{ 16 };

	constexpr std::size_t DisplayResolutionWidth{ 64 };
	constexpr std::size_t DisplayResolutionHeight{ 32 };

	/// SuperChip constants
	namespace schip
	{
		constexpr std::size_t NumberOfRPLFlags{ 8 };

		constexpr std::uint16_t FontsetAddress{ constants::FontsetAddress +
												constants::FontsetTotalByteSize };
		constexpr std::size_t FontsetCharByteSize{ 10 };
		constexpr std::size_t FontsetCharCount{ 16 };
		constexpr std::size_t FontsetTotalByteSize{ FontsetCharCount * FontsetCharByteSize };
		constexpr std::array<std::uint8_t, FontsetTotalByteSize> Fontset{
			// 0
			0b11111111,
			0b11111111,
			0b11000011,
			0b11000011,
			0b11000011,
			0b11000011,
			0b11000011,
			0b11000011,
			0b11111111,
			0b11111111,

			// 1
			0b00011100,
			0b01111100,
			0b11111100,
			0b00111100,
			0b00111100,
			0b00111100,
			0b00111100,
			0b00111100,
			0b11111111,
			0b11111111,

			// 2
			0b01111100,
			0b11101110,
			0b00000111,
			0b00000111,
			0b00001111,
			0b00011110,
			0b00111100,
			0b01111000,
			0b11111111,
			0b11111111,

			// 3
			0b01111100,
			0b11101110,
			0b00000111,
			0b00000111,
			0b00011111,
			0b00011111,
			0b00000111,
			0b00000111,
			0b11101110,
			0b01111100,

			// 4
			0b00000110,
			0b00111110,
			0b01101110,
			0b11001110,
			0b11111110,
			0b11111110,
			0b00001110,
			0b00001110,
			0b00001110,
			0b00001110,

			// 5
			0b11111111,
			0b11111111,
			0b11100000,
			0b11100000,
			0b11111100,
			0b11111110,
			0b00000111,
			0b00000111,
			0b11111110,
			0b11111100,

			// 6
			0b00111111,
			0b01111111,
			0b11100000,
			0b11100000,
			0b11111100,
			0b11111110,
			0b11100111,
			0b11100111,
			0b01111110,
			0b00111100,

			// 7
			0b11111111,
			0b11111111,
			0b00000111,
			0b00001111,
			0b00011110,
			0b00111100,
			0b01111000,
			0b11110000,
			0b11110000,
			0b11110000,

			// 8
			0b01111110,
			0b11100111,
			0b11000011,
			0b11100110,
			0b01111110,
			0b01111110,
			0b11100111,
			0b11000011,
			0b11100111,
			0b01111110,

			// 9
			0b00111100,
			0b01111110,
			0b11100111,
			0b11100111,
			0b01111111,
			0b00111111,
			0b00000111,
			0b00000111,
			0b11101110,
			0b01111100,

			// A
			0b00111100,
			0b01111110,
			0b11100111,
			0b11000011,
			0b11111111,
			0b11111111,
			0b11000011,
			0b11000011,
			0b11000011,
			0b11000011,

			// B
			0b11111100,
			0b11111110,
			0b11000111,
			0b11000011,
			0b11111110,
			0b11111110,
			0b11000011,
			0b11000111,
			0b11111110,
			0b11111100,

			// C
			0b00111111,
			0b01111111,
			0b11100000,
			0b11000000,
			0b11000000,
			0b11000000,
			0b11000000,
			0b11100000,
			0b01111111,
			0b00111111,

			// D
			0b11111100,
			0b11111110,
			0b11000111,
			0b11000011,
			0b11000011,
			0b11000011,
			0b11000011,
			0b11000111,
			0b11111110,
			0b11111100,

			// E
			0b11111111,
			0b11111111,
			0b11100000,
			0b11000000,
			0b11111111,
			0b11111111,
			0b11000000,
			0b11000000,
			0b11111111,
			0b11111111,

			// F
			0b11111111,
			0b11111111,
			0b11100000,
			0b11000000,
			0b11111111,
			0b11111111,
			0b11000000,
			0b11000000,
			0b11000000,
			0b11000000,
		};

		constexpr std::size_t ExtendedDisplayResolutionWidth{ 128 };
		constexpr std::size_t ExtendedDisplayResolutionHeight{ 64 };
	}
}