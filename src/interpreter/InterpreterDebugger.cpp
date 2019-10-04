#include "InterpreterDebugger.h"
#include <imgui.h>
#include <mutex>
#include <gsl/gsl_util>
#include "Icons.h"

static const ImVec4 SubtitleColor{ 0.1f, 0.8f, 0.05f, 1.0f };

// TODO: debugger GUI should resize to fit window

CInterpreterDebugger::CInterpreterDebugger(CInterpreter& interpreter)
	: CImGuiWindow("chip8-interpreter: Debugger"), mInterpreter(interpreter), mFirstDraw{ true }, mBreakpoints{},
	mDisassemblyGoToAddress{ InvalidDisassemblyGoToAddress }
{
}

void CInterpreterDebugger::Draw()
{
	CheckBreakpoints();

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
			constexpr std::size_t RegistersPerRow{ 4 };
			for (std::size_t j = 0; j < RegistersPerRow; j++)
			{
				ImGui::Text("V%X:  %02X", gsl::narrow<std::uint32_t>(i + 4 * j), c.V[i + 4 * j]);
				ImGui::NextColumn();
			}
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
				ImGui::Text("%04X", gsl::narrow<std::uint32_t>(i)); ImGui::NextColumn();
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

			constexpr std::size_t BytesPerLine{ 8 };
			constexpr std::size_t LineTotalCount{ SContext::MemorySize / BytesPerLine };
			ImGuiListClipper clipper(gsl::narrow<int>(LineTotalCount));
			while (clipper.Step())
			{
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
				{
					const std::size_t addr = i * BytesPerLine;
					ImGui::Text("%04X", gsl::narrow<std::uint32_t>(addr));
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

		// 'go to PC' button
		{
			const ImVec2 cursorPos = ImGui::GetCursorPos();
			ImGui::SameLine(ImGui::GetWindowWidth() - 28.0f);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2.0f);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(SubtitleColor));
			if (ImGui::Button(ICON_FA_CHEVRON_CIRCLE_RIGHT, ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing())))
			{
				mDisassemblyGoToAddress = mInterpreter.Context().PC;
			}
			ImGui::PopStyleColor(4);
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Go to PC");
			}
			ImGui::SetCursorPos(cursorPos);
		}

		ImGui::Separator();
		if (ImGui::BeginChild("DisassemblyLines", ImVec2(0.0f, 0.0f), false))
		{
			constexpr float LeftGapWidth = 20.0f;

			// left gap background
			{
				ImVec2 pos = ImGui::GetCursorScreenPos();
				ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(pos.x, 0.0f), ImVec2(pos.x + LeftGapWidth, std::numeric_limits<float>::max()), IM_COL32(50, 50, 50, 180));
			}

			constexpr std::size_t BytesPerLine{ CInterpreter::InstructionByteSize };
			constexpr std::size_t LineTotalCount{ SContext::MemorySize / BytesPerLine };

			// handle 'go to address' request
			if (mDisassemblyGoToAddress != InvalidDisassemblyGoToAddress)
			{
				const std::size_t lineIndex = mDisassemblyGoToAddress / BytesPerLine;
				const float offsetY = lineIndex * ImGui::GetTextLineHeightWithSpacing();
				ImGui::SetScrollY(offsetY);
				mDisassemblyGoToAddress = InvalidDisassemblyGoToAddress;
			}

			ImGuiListClipper clipper(gsl::narrow<int>(LineTotalCount));
			while (clipper.Step())
			{
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
				{
					ImGui::PushID(static_cast<int>(i));

					const std::size_t addr = i * BytesPerLine;

					// highlight background if the breakpoint is set
					if (mBreakpoints[i])
					{
						// text background
						ImVec2 pos = ImGui::GetCursorScreenPos();
						ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(pos.x + LeftGapWidth, pos.y), ImVec2(std::numeric_limits<float>::max(), pos.y + ImGui::GetTextLineHeight()), IM_COL32(200, 0, 0, 100));

						// circle icon
						const ImVec2 circleCenter{ pos.x + (LeftGapWidth * 0.5f), pos.y + ImGui::GetTextLineHeight() * 0.5f };
						const float circleRadius = ImGui::GetTextLineHeight() * 0.5f;
						ImGui::GetWindowDrawList()->AddCircleFilled(circleCenter, circleRadius, IM_COL32(240, 0, 0, 255));
						ImGui::GetWindowDrawList()->AddCircle(circleCenter, circleRadius, IM_COL32(255, 255, 255, 255));
					}

					// executing instruction indicator
					if (c.PC == addr)
					{
						ImVec2 prevCursor = ImGui::GetCursorPos();
						ImGui::SetCursorPos(ImVec2(prevCursor.x + LeftGapWidth * 0.25f, prevCursor.y));
						ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), ICON_FA_ANGLE_RIGHT);
						ImGui::SetCursorPos(prevCursor);
					}

					// breakpoint button
					if (ImGui::InvisibleButton("##breakpointButton", ImVec2(LeftGapWidth, ImGui::GetTextLineHeight())))
					{
						mBreakpoints[i] = !mBreakpoints[i];
					}
					ImGui::SameLine();

					if (addr >= CInterpreter::ProgramStartAddress)
					{
						ImGui::Text("%04X: ", gsl::narrow<std::uint32_t>(addr));
					}
					else
					{
						ImGui::TextDisabled("%04X: ", gsl::narrow<std::uint32_t>(addr));
					}
					ImGui::SameLine();

					std::uint16_t opcode = c.Memory[addr] << 8 | c.Memory[addr + 1];
					contextCopy.PC = gsl::narrow<std::uint16_t>(addr);
					contextCopy.IR = opcode;

					auto instOpt = mInterpreter.TryFindInstruction(opcode);
					if (instOpt.has_value())
					{
						const SInstruction& inst = instOpt.value().get();
						std::string instStr = inst.ToString(inst, contextCopy);
						ImGui::Text(instStr.c_str());
					}
					else
					{
						ImGui::TextDisabled("???");
					}

					ImGui::PopID();
				}
			}
		}
		ImGui::EndChild();
	}
	ImGui::EndChild();
}

void CInterpreterDebugger::CheckBreakpoints()
{
	if (mInterpreter.IsPaused())
	{
		return;
	}

	const std::size_t breakpointIndex = mInterpreter.Context().PC / 2;
	if (mBreakpoints[breakpointIndex])
	{
		mInterpreter.Pause(true);
	}
}
