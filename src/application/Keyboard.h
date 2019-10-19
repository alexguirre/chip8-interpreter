#pragma once
#include <SDL2/SDL.h>
#include <array>
#include <core/Context.h>
#include <cstdint>

class CKeyboard
{
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
	void GetState(c8::SKeyboardState& state) const;

private:
	static SDL_Scancode KeyToSdlScancode(std::uint8_t key);
};
