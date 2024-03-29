#include "Context.h"

namespace c8
{
	using namespace constants;

	SDisplay::SDisplay() { Reset(); }

	void SDisplay::Reset()
	{
		ExtendedMode = false;
		std::fill(PixelBuffer.begin(), PixelBuffer.end(), std::uint8_t(0));
	}

	SContext::SContext() { Reset(); }

	void SContext::Reset()
	{
		std::fill(V.begin(), V.end(), std::uint8_t(0));
		I = 0;
		PC = 0;
		SP = 0;
		DT = 0;
		ST = 0;
		IR = 0;
		std::fill(Stack.begin(), Stack.end(), std::uint16_t(0));
		std::fill(Memory.begin(), Memory.end(), std::uint8_t(0));
		Display.Reset();
		DisplayChanged = true;
		std::fill(Keyboard.begin(), Keyboard.end(), false);
		Exited = false;

		std::copy(Fontset.begin(), Fontset.end(), Memory.begin() + FontsetAddress);
		std::copy(schip::Fontset.begin(),
				  schip::Fontset.end(),
				  Memory.begin() + schip::FontsetAddress);
	}
}