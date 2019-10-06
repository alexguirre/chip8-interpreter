#pragma once
#include "ImGuiWindow.h"
#include <core/Interpreter.h>
#include <array>

class CInterpreterDebugger : public CImGuiWindow
{
private:
	static constexpr std::size_t InvalidDisassemblyGoToAddress = ~0u;

	c8::CInterpreter& mInterpreter;
	bool mFirstDraw;
	std::array<bool, (c8::constants::MemorySize / c8::constants::InstructionByteSize)> mBreakpoints;
	std::size_t mDisassemblyGoToAddress;

public:
	CInterpreterDebugger(c8::CInterpreter& interpreter);

	void Draw() override;

private:
	void DrawMenuBar();
	void DrawRegisters();
	void DrawStack();
	void DrawMemory();
	void DrawDisassembly();
	void CheckBreakpoints();
};
