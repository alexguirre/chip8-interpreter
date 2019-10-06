#include "AppPlatform.h"

CAppPlatform::CAppPlatform()
	: mDisplay{ std::make_unique<CDisplay>() }, mKeyboard{ std::make_unique<CKeyboard>() },
	mSound{ std::make_unique<CSound>() }
{
}

void CAppPlatform::GetKeyboardState(c8::SKeyboardState& dest)
{
	mKeyboard->GetState(dest);
}

void CAppPlatform::UpdateDisplay(const c8::SDisplayPixelBuffer& pixelBuffer)
{
	mDisplay->UpdatePixelBuffer(pixelBuffer);
}

void CAppPlatform::Beep(double frequency, std::chrono::milliseconds duration)
{
	mSound->Beep(frequency, duration);
}
