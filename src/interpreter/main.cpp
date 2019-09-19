#include <iostream>
//#include <SDL2/SDL.h>
#include <vector>

#include "Memory.h"
#include "Disassembler.h"


int main(int /*argc*/, char* /*argv*/[])
{
	CMemory mem;
	std::vector<std::uint8_t> data;
	data.reserve(256 * 2);
	for (std::uint8_t i = 0; i < 255; i++)
	{
		data.push_back(i);
		data.push_back(i);
	}
	mem.BulkWrite(0, data.begin(), data.end());
	CDisassembler d(mem);

	for (memptr addr = 0; addr < data.size(); addr += 2)
	{
		std::cout << d.DisassembleInstruction(addr) << "\n";
	}

	std::cout << "==================\n";

	std::array<uint8_t, 16> readBuffer;
	mem.BulkRead(0x10, readBuffer.size(), readBuffer.begin());
	for (int i = 0; i < readBuffer.size(); i += 2)
	{
		char buffer[256];
		sprintf_s(buffer, "%02X %02X\n", readBuffer[i], readBuffer[i + 1]);
		std::cout << buffer;
	}


	return 0;
}
