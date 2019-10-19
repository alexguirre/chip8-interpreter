#pragma once
#include "TypeAliases.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace c8
{
	struct SContext;
	struct SOpCode;
	struct SInstruction;

	using FInstructionHandler = std::function<void(SContext&)>;
	using FInstructionToString = std::function<std::string(const SInstruction&, SOpCode)>;

	struct SInstruction
	{
		std::string Name;
		FInstructionHandler Handler;
		std::uint16_t Opcode;
		std::uint16_t OpcodeMask;
		FInstructionToString ToString;

		static const std::vector<SInstruction> InstructionSet;

		static const SInstruction& FindInstruction(std::uint16_t opcode);
		static optional_cref<SInstruction> TryFindInstruction(std::uint16_t opcode);
	};
}
