#include "Disassembler.h"
#include <cstdio>

CDisassembler::CDisassembler(const CMemory& memory)
	: mMemory(memory)
{
}

std::string CDisassembler::DisassembleInstruction(memptr addr) const
{
	const std::uint8_t hi = mMemory.Read(addr);
	const std::uint8_t lo = mMemory.Read(addr + 1);

	const std::size_t stringSize = std::snprintf(nullptr, 0, "%04X\t%02X %02X", static_cast<std::uint16_t>(addr), hi, lo);
	std::string s;
	s.resize(stringSize);
	std::snprintf(s.data(), stringSize + 1, "%04X\t%02X %02X", static_cast<std::uint16_t>(addr), hi, lo);
	return s;
}