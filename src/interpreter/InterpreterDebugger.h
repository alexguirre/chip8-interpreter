#pragma once
#include "ImGuiWindow.h"
#include "Interpreter.h"

class CInterpreterDebugger : public CImGuiWindow
{
private:
	CInterpreter& mInterpreter;
	bool mFirstDraw;

public:
	CInterpreterDebugger(CInterpreter& interpreter);

	void Draw() override;

private:
	void DrawMenuBar();
	void DrawRegisters();
	void DrawStack();
	void DrawMemory();
	void DrawDisassembly();
};
