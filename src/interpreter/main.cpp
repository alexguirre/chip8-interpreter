#include <iostream>
//#include <SDL2/SDL.h>
#include <vector>

#include "Interpreter.h"

int main(int /*argc*/, char* /*argv*/[])
{
	CInterpreter interpreter;
	interpreter.LoadProgram(R"(E:\programming\chip8\roms\demos\Particle Demo [zeroZshadow, 2008].ch8)");

	for (memptr_t addr = 0; addr < CMemory::SizeInBytes - 8; addr += 8)
	{
		const CMemory& mem = interpreter.Memory();
		char buffer[256];
		sprintf_s(buffer, "%04X\t%02X %02X %02X %02X   %02X %02X %02X %02X\n",
			addr,
			mem.Read(addr + 0), mem.Read(addr + 1), mem.Read(addr + 2), mem.Read(addr + 3),
			mem.Read(addr + 4), mem.Read(addr + 5), mem.Read(addr + 6), mem.Read(addr + 7));
		std::cout << buffer;
	}

	return 0;
}
