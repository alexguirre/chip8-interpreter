#pragma once
#include <tuple>
#include <cstdint>
#include <array>
#include <SDL2/SDL.h>

class CDisplay
{
public:
	static constexpr std::size_t ResolutionWidth{ 64 };
	static constexpr std::size_t ResolutionHeight{ 32 };
	static constexpr std::size_t DefaultWindowWidth{ ResolutionWidth * 15 };
	static constexpr std::size_t DefaultWindowHeight{ ResolutionHeight * 15 };
	
	using PixelBuffer = std::array<std::uint8_t, (ResolutionWidth * ResolutionHeight)>;
	static constexpr std::size_t PixelBufferCount{ 3 }; // Number of pixel buffers, >1 to reduce flickering

	using RGBA = std::tuple<std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t>;
	static constexpr RGBA ForeColor{ std::uint8_t{0}, std::uint8_t{100}, std::uint8_t{0}, std::uint8_t{255} };
	static constexpr RGBA BackColor{ std::uint8_t{0}, std::uint8_t{0}, std::uint8_t{0}, std::uint8_t{255} };

private:
	SDL_Window* mWindow;
	SDL_Renderer* mRenderer;
	std::array<PixelBuffer, PixelBufferCount> mPixelBuffers;
	std::size_t mNextPixelBuffer;

public:
	CDisplay();
	~CDisplay();

	CDisplay(const CDisplay&) = delete;
	CDisplay& operator=(const CDisplay&) = delete;
	CDisplay(CDisplay&&) = default;
	CDisplay& operator=(CDisplay&&) = default;

	void Render();
	void UpdatePixelBuffer(const PixelBuffer& src);
};
