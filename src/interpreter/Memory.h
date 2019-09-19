#pragma once
#include <cstdint>
#include <array>
#include <algorithm>

using memptr = uint16_t;

class CMemory
{
public:
	static constexpr std::size_t SizeInBytes = 4096;

private:
	std::array<std::uint8_t, SizeInBytes> mBuffer;

public:
	std::uint8_t Read(memptr addr) const;
	void Write(memptr addr, std::uint8_t value);

	template<class OutputIt>
	OutputIt BulkRead(memptr addr, std::size_t length, OutputIt dest)
	{
		return std::copy(
			std::next(mBuffer.begin(), addr),
			std::next(mBuffer.begin(), addr + length),
			dest);
	}

	template<class InputIt>
	void BulkWrite(memptr addr, InputIt first, InputIt last)
	{
		std::copy(first, last, std::next(mBuffer.begin(), addr));
	}
};
