#include <iostream>
#include <vector>
#include <tclap/CmdLine.h>
#include <gsl/gsl_util>
#include "Interpreter.h"
#include "InterpreterDebugger.h"

int main(int argc, char* argv[])
{
	TCLAP::CmdLine cmd("Chip-8 interpreter", ' ', "WIP");
	TCLAP::UnlabeledValueArg<std::string> inputArg("input_file", "Specifies the filename of the program ROM.", true, "", "input_file");

	cmd.add(inputArg);

	cmd.parse(argc, argv);

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	auto sdlQuit = gsl::finally(&SDL_Quit);

	try
	{
		CInterpreter interpreter;
		interpreter.LoadProgram(inputArg.getValue());

		CInterpreterDebugger debugger{ interpreter };

		bool quit = false;
		while (!quit)
		{
			SDL_Event e;
			while (SDL_PollEvent(&e))
			{
				if (e.type == SDL_QUIT || (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE))
				{
					quit = true;
				}
				else if (e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_F5)
				{
					interpreter.SaveState("save.ch8save");
				}
				else if (e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_F8)
				{
					interpreter.LoadState("save.ch8save");
				}

				debugger.ProcessEvent(e);
			}

			debugger.Render();

			interpreter.Update();
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
