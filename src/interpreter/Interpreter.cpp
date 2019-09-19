#include "Interpreter.h"
#include <algorithm>
#include <fstream>
#include <iterator>

namespace fs = std::filesystem;

SContext::SContext()
{
	Reset();
}

void SContext::Reset()
{
	std::fill(V.begin(), V.end(), std::uint8_t(0));
	I = 0;
	PC = 0;
	SP = 0;
	DT = 0;
	ST = 0;
}

void CInterpreter::LoadProgram(const std::filesystem::path& filePath)
{
	if (!fs::is_regular_file(filePath))
	{
		throw std::invalid_argument("Path '" + filePath.string() + "' is an invalid file");
	}

	mMemory.Clear();

	std::basic_ifstream<std::uint8_t> file(filePath, std::ios::in | std::ios::binary);
	mMemory.BulkWrite(
		ProgramStartAddress,
		std::istreambuf_iterator(file),
		std::istreambuf_iterator<std::uint8_t>()
	);

	mContext.Reset();
}
