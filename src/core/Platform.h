#pragma once
#include <chrono>
#include "Context.h"

namespace c8
{
	class CPlatform
	{
	public:
		virtual ~CPlatform() = default;

		virtual void GetKeyboardState(SKeyboardState& dest) = 0;
		virtual void UpdateDisplay(const SDisplayPixelBuffer& pixelBuffer) = 0;
		virtual void Beep(double frequency, std::chrono::milliseconds duration) = 0;
	};
}