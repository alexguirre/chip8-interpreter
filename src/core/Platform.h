#pragma once
#include <chrono>
#include "Context.h"

namespace c8
{
	class IPlatform
	{
	public:
		virtual ~IPlatform() = default;

		virtual void GetKeyboardState(SKeyboardState& dest) = 0;
		virtual void UpdateDisplay(const SDisplay& display) = 0;
		virtual void Beep(double frequency, std::chrono::milliseconds duration) = 0;
	};
}