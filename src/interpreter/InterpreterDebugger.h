#pragma once
#include "ImGuiWindow.h"
#include "Interpreter.h"
#include <array>

class CInterpreterDebugger : public CImGuiWindow
{
private:
	CInterpreter& mInterpreter;
	bool mFirstDraw;
	std::array<bool, (SContext::MemorySize / 2)> mBreakpoints;

public:
	CInterpreterDebugger(CInterpreter& interpreter);

	void Draw() override;

private:
	void DrawMenuBar();
	void DrawRegisters();
	void DrawStack();
	void DrawMemory();
	void DrawDisassembly();
	void CheckBreakpoints();
};
