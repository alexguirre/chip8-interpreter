#pragma once
#include <cstdint>
#include <array>
#include "Constants.h"

namespace c8
{
	using SKeyboardState = std::array<bool, constants::KeyboardKeyCount>;
	using SDisplayPixelBuffer = std::array<std::uint8_t, (constants::DisplayResolutionWidth * constants::DisplayResolutionHeight)>;

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
		SDisplayPixelBuffer PixelBuffer;
		bool PixelBufferDirty;
		SKeyboardState Keyboard;

		SContext();

		void Reset();

		inline std::uint8_t X() const { return (IR & 0x0F00) >> 8; }
		inline std::uint8_t Y() const { return (IR & 0x00F0) >> 4; }
		inline std::uint16_t NNN() const { return (IR & 0x0FFF); }
		inline std::uint8_t KK() const { return (IR & 0x00FF); }
		inline std::uint8_t N() const { return (IR & 0x000F); }
	};
}