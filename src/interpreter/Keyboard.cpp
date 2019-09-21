#include "Keyboard.h"
#include <stdexcept>

CKeyboard::CKeyboard()
	: mState{SDL_GetKeyboardState(nullptr)}
{
}

bool CKeyboard::IsDown(std::uint8_t key) const
{
	return mState[KeyToSdlScancode(key)];
}

bool CKeyboard::IsUp(std::uint8_t key) const
{
	return !IsDown(key);
}

void CKeyboard::GetState(State& state) const
{
	for (std::uint8_t i = 0; i < NumberOfKeys; i++)
	{
		state[i] = IsDown(i);
	}
}

SDL_Scancode CKeyboard::KeyToSdlScancode(std::uint8_t key)
{
	switch (key)
	{
	// first row
	case 0x1: return SDL_SCANCODE_1;
	case 0x2: return SDL_SCANCODE_2;
	case 0x3: return SDL_SCANCODE_3;
	case 0xC: return SDL_SCANCODE_4;

	// second row
	case 0x4: return SDL_SCANCODE_Q;
	case 0x5: return SDL_SCANCODE_W;
	case 0x6: return SDL_SCANCODE_E;
	case 0xD: return SDL_SCANCODE_R;

	// third row
	case 0x7: return SDL_SCANCODE_A;
	case 0x8: return SDL_SCANCODE_S;
	case 0x9: return SDL_SCANCODE_D;
	case 0xE: return SDL_SCANCODE_F;

	// fourth row
	case 0xA: return SDL_SCANCODE_Z;
	case 0x0: return SDL_SCANCODE_X;
	case 0xB: return SDL_SCANCODE_C;
	case 0xF: return SDL_SCANCODE_V;
	}

	throw std::invalid_argument("Invalid key");
}
