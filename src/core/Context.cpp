#include "Context.h"

namespace c8
{
	SDisplay::SDisplay()
	{
		Reset();
	}

	void SDisplay::Reset()
	{
		ExtendedMode = false;
		std::fill(PixelBuffer.begin(), PixelBuffer.end(), std::uint8_t(0));
		PixelBufferDirty = true;
	}

	SContext::SContext()
	{
		Reset();
	}

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
		std::fill(Keyboard.begin(), Keyboard.end(), false);

		std::copy(constants::Fontset.begin(), constants::Fontset.end(), Memory.begin());
	}
}