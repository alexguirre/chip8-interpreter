#pragma once
#include <cstdint>
#include <array>
#include <SDL2/SDL.h>

class CKeyboard
{
public:
	static constexpr std::size_t NumberOfKeys = 16;
	using State = std::array<bool, CKeyboard::NumberOfKeys>;

private:
	const std::uint8_t* const mState;

public:
	CKeyboard();

	CKeyboard(const CKeyboard&) = delete;
	CKeyboard& operator=(const CKeyboard&) = delete;
	CKeyboard(CKeyboard&&) = default;
	CKeyboard& operator=(CKeyboard&&) = default;

	bool IsDown(std::uint8_t key) const;
	bool IsUp(std::uint8_t key) const;
	void GetState(State& state) const;

private:
	static SDL_Scancode KeyToSdlScancode(std::uint8_t key);
};
