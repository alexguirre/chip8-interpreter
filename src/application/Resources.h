#pragma once
#include <array>

class CResources
{
private:
	CResources() = delete;

public:
	static const std::array<std::uint32_t, 136544 / 4> FontAwesomeCompressedTTF;
};

