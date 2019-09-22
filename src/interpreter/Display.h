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
	
	using PixelBuffer = std::array<std::uint8_t, (ResolutionWidth * ResolutionHeight)>;

	using RGBA = std::tuple<std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t>;
	static constexpr RGBA PrimaryColor{ std::uint8_t{0}, std::uint8_t{100}, std::uint8_t{0}, std::uint8_t{255} };
	static constexpr RGBA SecondaryColor{ std::uint8_t{0}, std::uint8_t{0}, std::uint8_t{0}, std::uint8_t{255} };

private:
	SDL_Window* mWindow;
	SDL_Renderer* mRenderer;
	PixelBuffer mPixelBuffer;

public:
	CDisplay();
	~CDisplay();

	CDisplay(const CDisplay&) = delete;
	CDisplay& operator=(const CDisplay&) = delete;
	CDisplay(CDisplay&&) = default;
	CDisplay& operator=(CDisplay&&) = default;

	void Update();
	void UpdatePixelBuffer(const PixelBuffer& src);
};
