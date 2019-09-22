#include <iostream>
#include <vector>
#include <tclap/CmdLine.h>
#include "Interpreter.h"

int main(int argc, char* argv[])
{
	TCLAP::CmdLine cmd("Chip-8 interpreter", ' ', "WIP");
	TCLAP::UnlabeledValueArg<std::string> inputArg("input_file", "Specifies the filename of the program ROM.", true, "", "input_file");

	cmd.add(inputArg);

	cmd.parse(argc, argv);

	SDL_Init(SDL_INIT_VIDEO);

	try
	{
		CInterpreter interpreter;
		interpreter.LoadProgram(inputArg.getValue());
	
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
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	SDL_Quit();

	return 0;
}
