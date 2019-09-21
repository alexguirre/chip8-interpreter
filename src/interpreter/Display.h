#pragma once
#include <tuple>
#include <cstdint>
#include <array>
#include <SDL2/SDL.h>

class CDisplay
{
public:
	static constexpr std::tuple<std::int32_t, std::int32_t> Resolution{ 64, 32 };
	
	using PixelBuffer = std::array<std::uint8_t, (std::get<0>(Resolution) * std::get<1>(Resolution))>;

	using Color = std::tuple<std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t>;
	static constexpr Color PrimaryColor{ std::uint8_t{0}, std::uint8_t{100}, std::uint8_t{0}, std::uint8_t{255} };
	static constexpr Color SecondaryColor{ std::uint8_t{0}, std::uint8_t{0}, std::uint8_t{0}, std::uint8_t{255} };

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
