#include "Memory.h"

std::uint8_t CMemory::Read(memptr addr) const
{
	return mBuffer[addr];
}

void CMemory::Write(memptr addr, std::uint8_t value)
{
	mBuffer[addr] = value;
}
