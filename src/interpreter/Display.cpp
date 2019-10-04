#include "Display.h"
#include <stdexcept>
#include <algorithm>

CDisplay::CDisplay()
	: mPixelBuffers{}, mNextPixelBuffer{ 0 }
{
	mWindow = SDL_CreateWindow(
		"chip8-interpreter",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		DefaultWindowWidth, DefaultWindowHeight,
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

	SDL_RenderSetLogicalSize(mRenderer, ResolutionWidth, ResolutionHeight);
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

	constexpr std::size_t MaxRects{ ResolutionWidth * ResolutionHeight };
	std::array<SDL_Rect, MaxRects> rects;
	std::size_t rectCount = 0;

	for (std::int32_t y = 0; y < ResolutionHeight; y++)
	{
		for (std::int32_t x = 0; x < ResolutionWidth; x++)
		{
			const std::size_t pixelIndex = x + y * ResolutionWidth;
			
			// check if the pixel is set in any of the buffers
			if (std::any_of(mPixelBuffers.begin(), mPixelBuffers.end(), [pixelIndex](const auto& buffer)
				{
					return buffer[pixelIndex];
				}))
			{
				rects[rectCount++] = {
					x, y, 1, 1
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
		mPixelBuffers[mNextPixelBuffer].begin()
	);

	// set the buffer to update next
	mNextPixelBuffer = (mNextPixelBuffer + 1) % mPixelBuffers.size();
}
