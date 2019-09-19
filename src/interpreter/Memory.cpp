#include "Memory.h"

CMemory::CMemory()
{
	Clear();
}

std::uint8_t CMemory::Read(memptr_t addr) const
{
	return mBuffer[addr];
}

void CMemory::Write(memptr_t addr, std::uint8_t value)
{
	mBuffer[addr] = value;
}

void CMemory::BulkWrite(memptr_t addr, std::size_t length, std::uint8_t value)
{
	std::fill(
		std::next(mBuffer.begin(), addr),
		std::next(mBuffer.begin(), addr + length),
		value
	);
}

void CMemory::Clear()
{
	BulkWrite(0, SizeInBytes, 0);
}
