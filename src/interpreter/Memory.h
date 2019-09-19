#pragma once
#include <cstdint>
#include <array>
#include <algorithm>

using memptr_t = uint16_t;

class CMemory
{
public:
	static constexpr std::size_t SizeInBytes = 4096;

private:
	std::array<std::uint8_t, SizeInBytes> mBuffer;

public:
	CMemory();
	
	CMemory(CMemory&&) = default;
	CMemory& operator=(CMemory&&) = default;

	CMemory(const CMemory&) = delete;
	CMemory& operator=(const CMemory&) = delete;

	std::uint8_t Read(memptr_t addr) const;
	void Write(memptr_t addr, std::uint8_t value);

	template<class OutputIt>
	OutputIt BulkRead(memptr_t addr, std::size_t length, OutputIt dest) const
	{
		return std::copy(
			std::next(mBuffer.begin(), addr),
			std::next(mBuffer.begin(), addr + length),
			dest
		);
	}

	template<class InputIt>
	void BulkWrite(memptr_t addr, InputIt first, InputIt last)
	{
		std::copy(first, last, std::next(mBuffer.begin(), addr));
	}

	void BulkWrite(memptr_t addr, std::size_t length, std::uint8_t value);

	// Writes 0 in all the memory buffer.
	void Clear();
};
