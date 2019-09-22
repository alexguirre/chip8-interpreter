#include "Display.h"
#include <stdexcept>

CDisplay::CDisplay()
	: mPixelBuffer()
{
	mWindow = SDL_CreateWindow(
		"chip8-interpreter",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		ResolutionWidth * 15, ResolutionHeight * 15,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);

	if (!mWindow)
	{
		throw std::runtime_error("Failed to create window: " + std::string(SDL_GetError()));
	}

	mRenderer = SDL_CreateRenderer(
		mWindow, -1,
		SDL_RENDERER_ACCELERATED
	);

	if (!mRenderer)
	{
		throw std::runtime_error("Failed to create renderer: " + std::string(SDL_GetError()));
	}
}

CDisplay::~CDisplay()
{
	if (mRenderer)
	{
		SDL_DestroyRenderer(mRenderer);
	}

	if (mWindow)
	{
		SDL_DestroyWindow(mWindow);
	}
}

void CDisplay::Update()
{
	SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);
	SDL_RenderClear(mRenderer);

	int w, h;
	SDL_GetWindowSize(mWindow, &w, &h);

	const std::size_t pixelW = w / ResolutionWidth;
	const std::size_t pixelH = h / ResolutionHeight;

	for (std::size_t y = 0; y < ResolutionHeight; y++)
	{
		for (std::size_t x = 0; x < ResolutionWidth; x++)
		{
			SDL_Rect pixel{
				x * pixelW, y * pixelH,
				pixelW, pixelH
			};

			RGBA c = (mPixelBuffer[x + y * ResolutionWidth]) ? PrimaryColor : SecondaryColor;
			SDL_SetRenderDrawColor(mRenderer, 
				std::get<0>(c), std::get<1>(c), std::get<2>(c), std::get<3>(c));
			SDL_RenderFillRect(mRenderer, &pixel);
		}
	}

	SDL_RenderPresent(mRenderer);
}

void CDisplay::UpdatePixelBuffer(const PixelBuffer& src)
{
	std::copy(
		src.begin(),
		src.end(),
		mPixelBuffer.begin()
	);
}
