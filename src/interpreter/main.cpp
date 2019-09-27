#include <iostream>
#include <vector>
#include <thread>
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
		interpreter.Pause(true);

		CInterpreterDebugger debugger{ interpreter };

		bool quit = false;

		// TODO: CInterpreter is not fully thread-safe
		std::thread interpreterThread([&interpreter, &quit]()
			{
				while (!quit)
				{
					std::this_thread::yield();

					interpreter.Update();
				}
			}
		);

		while (!quit)
		{
			std::this_thread::yield();

			SDL_Event e;
			while (SDL_PollEvent(&e))
			{
				if (e.type == SDL_QUIT || (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE))
				{
					quit = true;
				}
				else if (e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_F5)
				{
					// Pause because CInterpreter is not thread-safe so it could try modify the state
					// while saving it. Not the ideal solution, could still cause issues.
					interpreter.Pause(true);
					interpreter.SaveState("save.ch8save");
					interpreter.Pause(false);
				}
				else if (e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_F8)
				{
					// Pause because CInterpreter is not thread-safe so it could try to execute
					// the program while loading the state. Not the ideal solution, could still cause issues.
					interpreter.Pause(true);
					interpreter.LoadState("save.ch8save");
					interpreter.Pause(false);
				}

				debugger.ProcessEvent(e);
			}

			debugger.Render();
			interpreter.RenderDisplay();
		}

		interpreterThread.join();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
