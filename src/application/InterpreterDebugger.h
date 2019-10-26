#pragma once
#include "ImGuiWindow.h"
#include <core/Interpreter.h>
#include <vector>

class CInterpreterDebugger : public CImGuiWindow
{
private:
	static constexpr std::size_t InvalidDisassemblyGoToAddress = ~0u;

	c8::CInterpreter& mInterpreter;
	bool mFirstDraw;
	std::vector<std::uint16_t> mBreakpoints;
	std::size_t mDisassemblyGoToAddress;

public:
	CInterpreterDebugger(c8::CInterpreter& interpreter);

	bool IsBreakpointSet(std::uint16_t address) const;
	void SetBreakpoint(std::uint16_t address, bool enable);

	void Draw() override;

private:
	void DrawMenuBar();
	void DrawRegisters();
	void DrawStack();
	void DrawMemory();
	void DrawDisassembly();
	void CheckBreakpoints();
};
