#pragma once
#include "Context.h"
#include <cstdint>
#include <string>
#include <vector>

namespace c8
{
	struct SInstruction;

	struct SDisassemblyLine
	{
		std::size_t Address;
		SOpCode OpCode;
		std::string Source;

		inline SDisassemblyLine(std::size_t address, SOpCode opCode, std::string source)
			: Address(address), OpCode(opCode), Source(std::move(source))
		{
		}
	};

	using SDisassembly = std::vector<SDisassemblyLine>;

	class CDisassembler
	{
	public:
		SDisassembly Disassemble(std::size_t startAddress, const SMemory& memory) const;

	private:
		void Disassemble(std::size_t addr, const SMemory& mem, SDisassembly& dest) const;
	};
}