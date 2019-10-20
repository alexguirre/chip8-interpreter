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

	enum class EInstructionKind
	{
		Other = 0,  // An instruction that does not belong to any of the other kinds
		Jump = 1,   // An unconditional jump instruction
		Branch = 2, // A conditional jump instruction
		Return = 3, // The RET instruction
		Exit = 4,   // The EXIT instruction
	};

	struct SInstruction
	{
		std::string Name;
		FInstructionHandler Handler;
		std::uint16_t Opcode;
		std::uint16_t OpcodeMask;
		FInstructionToString ToString;
		EInstructionKind Kind;

		static const std::vector<SInstruction> InstructionSet;

		static const SInstruction& FindInstruction(std::uint16_t opcode);
		static optional_cref<SInstruction> TryFindInstruction(std::uint16_t opcode);
	};
}
