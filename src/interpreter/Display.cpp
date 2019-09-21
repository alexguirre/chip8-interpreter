#include "Display.h"
#include <stdexcept>

CDisplay::CDisplay()
	: mPixelBuffer()
{
	mWindow = SDL_CreateWindow(
		"chip8-interpreter",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		std::get<0>(Resolution) * 15, std::get<1>(Resolution) * 15,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);

	if (!mWindow)
	{
		throw std::runtime_error("Failed to created window: " + std::string(SDL_GetError()));
	}

	mRenderer = SDL_CreateRenderer(
		mWindow, -1,
		SDL_RENDERER_ACCELERATED
	);

	if (!mRenderer)
	{
		throw std::runtime_error("Failed to created renderer: " + std::string(SDL_GetError()));
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

	constexpr std::int32_t resX = std::get<0>(Resolution);
	constexpr std::int32_t resY = std::get<1>(Resolution);

	const std::int32_t pixelW = w / resX;
	const std::int32_t pixelH = h / resY;

	for (std::int32_t y = 0; y < resY; y++)
	{
		for (std::int32_t x = 0; x < resX; x++)
		{
			SDL_Rect pixel{
				x * pixelW, y * pixelH,
				pixelW, pixelH
			};

			Color c = (mPixelBuffer[x + y * resX]) ? PrimaryColor : SecondaryColor;
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
