#pragma once
#include "Constants.h"
#include <array>
#include <cstdint>

namespace c8
{
	using SKeyboardState = std::array<bool, constants::KeyboardKeyCount>;
	using SDisplayPixelBuffer = std::array<std::uint8_t,
										   (constants::schip::ExtendedDisplayResolutionWidth *
											constants::schip::ExtendedDisplayResolutionHeight)>;

	struct SDisplay
	{
		bool ExtendedMode;
		SDisplayPixelBuffer PixelBuffer;

		SDisplay();

		void Reset();

		inline std::size_t Width() const
		{
			return ExtendedMode ? constants::schip::ExtendedDisplayResolutionWidth :
								  constants::DisplayResolutionWidth;
		}
		inline std::size_t Height() const
		{
			return ExtendedMode ? constants::schip::ExtendedDisplayResolutionHeight :
								  constants::DisplayResolutionHeight;
		}
	};

	struct SOpCode
	{
		std::uint16_t Value;

		inline SOpCode() : Value(0) {}
		inline SOpCode(std::uint16_t value) : Value(value) {}

		inline operator std::uint16_t() const { return Value; }

		inline std::uint8_t X() const { return (Value & 0x0F00) >> 8; }
		inline std::uint8_t Y() const { return (Value & 0x00F0) >> 4; }
		inline std::uint16_t NNN() const { return (Value & 0x0FFF); }
		inline std::uint8_t KK() const { return (Value & 0x00FF); }
		inline std::uint8_t N() const { return (Value & 0x000F); }
	};

	struct SContext
	{
		std::array<std::uint8_t, constants::NumberOfRegisters> V; // General purpose registers
		std::uint16_t I;                                          // The memory address register
		std::uint16_t PC;                                         // The program counter
		std::uint8_t SP;                                          // The stack pointer
		std::uint8_t DT;                                          // The delay timer
		std::uint8_t ST;                                          // The sound timer
		SOpCode IR;                                               // The current instruction opcode
		std::array<std::uint16_t, constants::StackSize> Stack;
		std::array<std::uint8_t, constants::MemorySize> Memory;
		std::array<std::uint8_t, constants::schip::NumberOfRPLFlags> R; // RPL user flags
		SDisplay Display;
		bool DisplayChanged;
		SKeyboardState Keyboard;
		bool Exited;

		SContext();

		void Reset();

		inline std::uint8_t X() const { return IR.X(); }
		inline std::uint8_t Y() const { return IR.Y(); }
		inline std::uint16_t NNN() const { return IR.NNN(); }
		inline std::uint8_t KK() const { return IR.KK(); }
		inline std::uint8_t N() const { return IR.N(); }
	};
}