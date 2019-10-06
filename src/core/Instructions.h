#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <functional>

namespace c8
{
	struct SContext;
	struct SInstruction;

	using FInstructionHandler = std::function<void(SContext&)>;
	using FInstructionToString = std::function<std::string(const SInstruction&, const SContext&)>;

	struct SInstruction
	{
		std::string Name;
		FInstructionHandler Handler;
		std::uint16_t Opcode;
		std::uint16_t OpcodeMask;
		FInstructionToString ToString;

		static const std::vector<SInstruction> InstructionSet;
	};
}
