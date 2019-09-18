#include <iostream>
#include <SDL2/SDL.h>

int main(int argc, char* argv[])
{
	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window* window = SDL_CreateWindow(
		"The window",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		640, 480,
		SDL_WINDOW_OPENGL);

	if (!window)
	{
		std::cerr << "Failed to create window\n";
		return 1;
	}

	SDL_Delay(5000);

	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
}
