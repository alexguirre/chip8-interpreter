#pragma once
#include <SDL2/SDL.h>
#include <memory>

class CImGuiWindow
{
private:
	class Impl;
	std::unique_ptr<Impl> mImpl;

public:
	CImGuiWindow();
	~CImGuiWindow();

	bool ProcessEvent(const SDL_Event& event);
	void Render();

	virtual void Draw() = 0;
};
