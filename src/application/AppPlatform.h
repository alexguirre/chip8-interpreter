#pragma once
#include <core/Platform.h>
#include "Display.h"
#include "Keyboard.h"
#include "Sound.h"

class CAppPlatform : public c8::IPlatform
{
private:
	std::unique_ptr<CDisplay> mDisplay;
	std::unique_ptr<CKeyboard> mKeyboard;
	std::unique_ptr<CSound> mSound;

public:
	CAppPlatform();

	CAppPlatform(const CAppPlatform&) = delete;
	CAppPlatform& operator=(const CAppPlatform&) = delete;

	CAppPlatform(CAppPlatform&&) = default;
	CAppPlatform& operator=(CAppPlatform&&) = default;

	inline CDisplay& Display() { return *mDisplay; }
	inline CKeyboard& Keyboard() { return *mKeyboard; }
	inline CSound& Sound() { return *mSound; }
	
	void GetKeyboardState(c8::SKeyboardState& dest) override;
	void UpdateDisplay(const c8::SDisplay& display) override;
	void Beep(double frequency, std::chrono::milliseconds duration) override;
};
