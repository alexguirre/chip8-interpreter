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

void CDisplay::Render()
{
	SDL_SetRenderDrawColor(mRenderer,
		std::get<0>(BackColor), std::get<1>(BackColor),
		std::get<2>(BackColor), std::get<3>(BackColor));
	SDL_RenderClear(mRenderer);

	int w, h;
	SDL_GetWindowSize(mWindow, &w, &h);

	const std::int32_t pixelW = w / ResolutionWidth;
	const std::int32_t pixelH = h / ResolutionHeight;

	constexpr std::size_t MaxRects{ ResolutionWidth * ResolutionHeight };
	std::array<SDL_Rect, MaxRects> rects;
	std::size_t rectCount = 0;

	for (std::int32_t y = 0; y < ResolutionHeight; y++)
	{
		for (std::int32_t x = 0; x < ResolutionWidth; x++)
		{
			if (mPixelBuffer[x + y * ResolutionWidth])
			{
				rects[rectCount++] = {
					x * pixelW, y * pixelH,
					pixelW, pixelH
				};
			}
		}
	}

	SDL_SetRenderDrawColor(mRenderer,
		std::get<0>(ForeColor), std::get<1>(ForeColor),
		std::get<2>(ForeColor), std::get<3>(ForeColor));
	SDL_RenderFillRects(mRenderer, rects.data(), static_cast<int>(rectCount));
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
