#pragma once
#include <cstdint>
#include <string>
#include "Memory.h"

class CMemory;

class CDisassembler
{
private:
	const CMemory& mMemory;

public:
	CDisassembler(const CMemory& memory);

	std::string DisassembleInstruction(memptr addr) const;
};

