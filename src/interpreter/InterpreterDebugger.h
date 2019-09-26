#pragma once
#include "ImGuiWindow.h"
#include "Interpreter.h"
#include <array>

class CInterpreterDebugger : public CImGuiWindow
{
private:
	static constexpr std::size_t InvalidDisassemblyGoToAddress = ~0u;

	CInterpreter& mInterpreter;
	bool mFirstDraw;
	std::array<bool, (SContext::MemorySize / 2)> mBreakpoints;
	std::size_t mDisassemblyGoToAddress;

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
