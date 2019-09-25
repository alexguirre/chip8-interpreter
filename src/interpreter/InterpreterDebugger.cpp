#include "InterpreterDebugger.h"
#include <imgui.h>
#include <mutex>
#include "Icons.h"

static const ImVec4 SubtitleColor{ 0.1f, 0.8f, 0.05f, 1.0f };

// TODO: debugger breakpoints

CInterpreterDebugger::CInterpreterDebugger(CInterpreter& interpreter)
	: CImGuiWindow("chip8-interpreter: Debugger"),	mInterpreter(interpreter), mFirstDraw{ true }
{
}

void CInterpreterDebugger::Draw()
{
	ImGuiIO& io = ImGui::GetIO();
	
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

	ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
	if (ImGui::Begin("Debugger", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize))
	{
		DrawMenuBar();
		DrawRegisters();
		DrawMemory();
		ImGui::SameLine();
		ImGui::SetCursorPosY(ImGui::GetWindowContentRegionMin().y);
		DrawStack();
		ImGui::SameLine();
		DrawDisassembly();
	}
	ImGui::End();

	ImGui::PopStyleVar();

	if (mFirstDraw)
	{
		mFirstDraw = false;
	}
}

void CInterpreterDebugger::DrawMenuBar()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::MenuItem(ICON_FA_PLAY" Continue", nullptr, false, mInterpreter.IsPaused()))
		{
			mInterpreter.Pause(false);
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Continue");
		}

		if (ImGui::MenuItem(ICON_FA_PAUSE, nullptr, false, !mInterpreter.IsPaused()))
		{
			mInterpreter.Pause(true);
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Break");
		}

		if (ImGui::MenuItem(ICON_FA_ARROW_RIGHT, nullptr, false, mInterpreter.IsPaused()))
		{
			mInterpreter.Step();
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Step");
		}

		ImGui::EndMenuBar();
	}
}

void CInterpreterDebugger::DrawRegisters()
{
	const SContext& c = mInterpreter.Context();

	if (ImGui::BeginChild("Registers1", ImVec2(300.0f, 100.0f), true))
	{
		ImGui::TextColored(SubtitleColor, "Registers");

		ImGui::Columns(4);
		for (std::size_t i = 0; i < c.V.size() / 4; i++)
		{
			ImGui::Text("V%X:  %02X", i + 4 * 0, c.V[i + 4 * 0]); ImGui::NextColumn();
			ImGui::Text("V%X:  %02X", i + 4 * 1, c.V[i + 4 * 1]); ImGui::NextColumn();
			ImGui::Text("V%X:  %02X", i + 4 * 2, c.V[i + 4 * 2]); ImGui::NextColumn();
			ImGui::Text("V%X:  %02X", i + 4 * 3, c.V[i + 4 * 3]); ImGui::NextColumn();
		}
		ImGui::Columns(1);
	}
	ImGui::EndChild();

	ImGui::SameLine();

	if (ImGui::BeginChild("Registers2", ImVec2(150.0f, 100.0f), true))
	{
		ImGui::TextColored(SubtitleColor, "Misc Registers");

		ImGui::Columns(2);
		ImGui::Text("PC: %04X", c.PC);  ImGui::NextColumn();
		ImGui::Text(" I: %04X", c.I);  ImGui::NextColumn();
		ImGui::Text("SP:   %02X", c.SP);  ImGui::NextColumn();
		ImGui::Text("IR: %04X", c.IR);  ImGui::NextColumn();
		ImGui::Text("DT:   %02X", c.DT);  ImGui::NextColumn();
		ImGui::NextColumn();
		ImGui::Text("ST:   %02X", c.ST);  ImGui::NextColumn();
		ImGui::NextColumn();
		ImGui::Columns(1);
	}
	ImGui::EndChild();
}

void CInterpreterDebugger::DrawStack()
{
	const SContext& c = mInterpreter.Context();

	if (ImGui::BeginChild("Stack", ImVec2(150.0f, 424.0f), true))
	{
		ImGui::TextColored(SubtitleColor, "Stack");

		ImGui::Separator();
		if (ImGui::BeginChild("StackValues", ImVec2(0.0f, 0.0f), false))
		{
			ImGui::Columns(2);
			ImGui::Text("Address"); ImGui::NextColumn();
			ImGui::Text("Value"); ImGui::NextColumn();
			ImGui::Separator();
			for (std::size_t i = 0; i < c.Stack.size(); i++)
			{
				ImGui::Text("%04X", i); ImGui::NextColumn();
				ImGui::Text("%04X", c.Stack[i]); ImGui::NextColumn();
			}
			ImGui::Columns(1);
		}
		ImGui::EndChild();
	}
	ImGui::EndChild();
}

void CInterpreterDebugger::DrawMemory()
{
	const SContext& c = mInterpreter.Context();

	if (ImGui::BeginChild("Memory", ImVec2(458.0f, 320.0f), true, ImGuiWindowFlags_NoScrollbar))
	{
		ImGui::TextColored(SubtitleColor, "Memory");

		ImGui::Separator();
		if (ImGui::BeginChild("MemoryBytes", ImVec2(0.0f, 0.0f), false))
		{
			ImGui::Columns(3);

			if (mFirstDraw)
			{
				ImGui::SetColumnWidth(0, 70.0f);
				ImGui::SetColumnWidth(1, 200.0f);
			}
			ImGui::Text("Address"); ImGui::NextColumn();
			ImGui::NextColumn();
			ImGui::NextColumn();
			ImGui::Separator();

			constexpr std::int32_t BytesPerLine{ 8 };
			constexpr std::int32_t LineTotalCount{ SContext::MemorySize / BytesPerLine };
			ImGuiListClipper clipper(LineTotalCount);
			while (clipper.Step())
			{
				for (std::size_t i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
				{
					std::size_t addr = i * BytesPerLine;
					ImGui::Text("%04X", addr);
					ImGui::NextColumn();

					// draw bytes
					for (std::size_t offset = 0; offset < BytesPerLine; offset++)
					{
						if (offset != 0) ImGui::SameLine();
						std::uint8_t byte = c.Memory[addr + offset];
						if (byte == 0)
						{
							ImGui::TextDisabled("00");
						}
						else
						{
							ImGui::Text("%02X", byte);
						}

						if (offset == (BytesPerLine / 2) - 1)
						{
							ImGui::SameLine();
							ImGui::Dummy(ImVec2(2.0f, 0.0f));
						}
					}
					ImGui::NextColumn();

					// draw ASCII
					auto getAscii = [](std::uint8_t b) { return (b >= 32 && b < 128) ? static_cast<char>(b) : '.'; };
					ImGui::Text("%c%c%c%c %c%c%c%c",
						getAscii(c.Memory[addr + 0]), getAscii(c.Memory[addr + 1]), getAscii(c.Memory[addr + 2]), getAscii(c.Memory[addr + 3]),
						getAscii(c.Memory[addr + 4]), getAscii(c.Memory[addr + 5]), getAscii(c.Memory[addr + 6]), getAscii(c.Memory[addr + 7]));
					ImGui::NextColumn();
				}
			}

			ImGui::Columns(1);
		}
		ImGui::EndChild();
	}
	ImGui::EndChild();
}

void CInterpreterDebugger::DrawDisassembly()
{
	SContext contextCopy = mInterpreter.Context(); // required for instructions ToString, TODO: optimize
	const SContext& c = mInterpreter.Context();

	if (ImGui::BeginChild("Disassembly", ImVec2(458.0f, 424.0f), true, ImGuiWindowFlags_NoScrollbar))
	{
		ImGui::TextColored(SubtitleColor, "Disassembly");

		ImGui::Separator();
		if (ImGui::BeginChild("DisassemblyLines", ImVec2(0.0f, 0.0f), false))
		{
			constexpr std::int32_t BytesPerLine{ 2 };
			constexpr std::int32_t LineTotalCount{ SContext::MemorySize / BytesPerLine };
			ImGuiListClipper clipper(LineTotalCount);
			while (clipper.Step())
			{
				for (std::size_t i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
				{
					std::size_t addr = i * BytesPerLine;

					// highlight background if we're at the current instruction
					if (c.PC == addr)
					{
						ImVec2 pos = ImGui::GetCursorScreenPos();
						ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0.0f, pos.y), ImVec2(std::numeric_limits<float>::max(), pos.y + ImGui::GetTextLineHeight()), IM_COL32(200, 0, 0, 100));
					}

					if (addr >= CInterpreter::ProgramStartAddress)
					{
						ImGui::Text("%04X: ", addr);
					}
					else
					{
						ImGui::TextDisabled("%04X: ", addr);
					}
					ImGui::SameLine();

					std::uint16_t opcode = c.Memory[addr] << 8 | c.Memory[addr + 1];
					contextCopy.PC = static_cast<std::uint16_t>(addr);
					contextCopy.IR = opcode;

					auto inst = mInterpreter.TryFindInstruction(opcode);
					if (inst.has_value())
					{
						std::string instStr = inst.value().get().ToString(contextCopy);
						ImGui::Text(instStr.c_str());
					}
					else
					{
						ImGui::TextDisabled("???");
					}
				}
			}
		}
		ImGui::EndChild();
	}
	ImGui::EndChild();
}