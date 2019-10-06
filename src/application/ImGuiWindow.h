#pragma once
#include <SDL2/SDL.h>
#include <memory>
#include <string>

class CImGuiWindow
{
private:
	class Impl;
	std::unique_ptr<Impl> mImpl;

public:
	CImGuiWindow(const std::string& title);
	~CImGuiWindow();

	bool ProcessEvent(const SDL_Event& event);
	void Render();

	virtual void Draw() = 0;
};
