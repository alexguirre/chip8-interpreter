#include <iostream>
#include <vector>

#include "Interpreter.h"

int main(int /*argc*/, char* /*argv*/[])
{
	SDL_Init(SDL_INIT_VIDEO);
	{
		CInterpreter interpreter;
		interpreter.LoadProgram(R"(E:\programming\chip8\roms\demos\Zero Demo [zeroZshadow, 2007].ch8)");
	
		bool quit = false;
		while (!quit)
		{
			SDL_Event e;
			while (SDL_PollEvent(&e))
			{
				if (e.type == SDL_QUIT)
				{
					quit = true;
				}
			}

			interpreter.Update();
		}
	}
	SDL_Quit();

	return 0;
}
