#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct SContext;

using FInstructionHandler = void(*)(SContext& context);
using FInstructionToString = std::string(*)(SContext& context);

struct SInstruction
{
	std::string Name;
	FInstructionHandler Handler;
	std::uint16_t Opcode;
	std::uint16_t OpcodeMask;
	FInstructionToString ToString;

	static const std::vector<SInstruction> InstructionSet;
};

