#pragma once
#include <array>
#include <cstdint>

struct SContext
{
	static constexpr std::size_t NumRegisters = 16;

	union // General purpose registers
	{
		struct
		{
			std::uint8_t V0, V1, V2, V3, V4, V5, V6, V7,
						V8, V9, VA, VB, VC, VD, VE, VF;
		};
		std::array<std::uint8_t, NumRegisters> V;
	};
	std::uint16_t I;	// The memory address register
	std::uint16_t PC;	// The program counter
	std::uint8_t SP;	// The stack pointer
	std::uint8_t DT;	// The delay timer
	std::uint8_t ST;	// The sound timer

	SContext();
	
	void Reset();
};

class CInterpreter
{
private:
	SContext mContext;

public:
	inline const SContext& Context() const { return mContext; }
};

