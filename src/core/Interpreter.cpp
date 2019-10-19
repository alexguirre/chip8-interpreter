#include "Interpreter.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <random>
#include <sstream>

namespace fs = std::filesystem;

namespace c8
{
	using namespace constants;

	CInterpreter::CInterpreter(const std::shared_ptr<IPlatform>& platform)
		: mPlatform{ platform }, mContext{}, mPaused{ false }
	{
	}

	void CInterpreter::Pause(bool pause) { mPaused = pause; }

	void CInterpreter::Update()
	{
		if (!IsPaused())
		{
			Step();
		}
	}

	void CInterpreter::Step()
	{
		if (mContext.Exited)
		{
			return;
		}

		mPlatform->GetKeyboardState(mContext.Keyboard);

		const auto now = Clock::now();

		if ((now - mLastCycleTime) >= CyclesRate)
		{
			DoCycle();
			mLastCycleTime = now;
		}

		if ((now - mLastTimerTickTime) >= TimersRate)
		{
			DoTimerTick();
			mLastTimerTickTime = now;
		}
	}

	void CInterpreter::DoCycle()
	{
		SContext& c = mContext;

		if (c.Exited)
		{
			return;
		}

		// fetch
		const std::uint16_t opcode = c.Memory[c.PC] << 8 | c.Memory[c.PC + std::size_t{ 1 }];
		c.IR = opcode;

		// move to next instruction
		c.PC += InstructionByteSize;

		// execute
		const SInstruction& instr = SInstruction::FindInstruction(opcode);
		instr.Handler(c);

		// update display
		if (mContext.DisplayChanged)
		{
			mPlatform->UpdateDisplay(mContext.Display);
			mContext.DisplayChanged = false;
		}
	}

	void CInterpreter::DoTimerTick()
	{
		if (mContext.Exited)
		{
			return;
		}

		if (mContext.DT > 0)
		{
			mContext.DT--;
		}

		if (mContext.ST > 0)
		{
			mContext.ST--;
			if (mContext.ST == 0)
			{
				DoBeep();
			}
		}
	}

	void CInterpreter::DoBeep() { mPlatform->Beep(BeepFrequency, BeepDuration); }

	void CInterpreter::LoadProgram(const std::filesystem::path& filePath)
	{
		if (!fs::is_regular_file(filePath))
		{
			throw std::invalid_argument("Path '" + filePath.string() + "' is an invalid file");
		}

		mContext.Reset();

		std::ifstream file(filePath, std::ios::in | std::ios::binary);
		std::copy(std::istreambuf_iterator(file),
				  std::istreambuf_iterator<char>(),
				  std::next(mContext.Memory.begin(), ProgramStartAddress));

		mContext.PC = ProgramStartAddress;
	}

	void CInterpreter::LoadState(const std::filesystem::path& filePath)
	{
		if (!fs::is_regular_file(filePath))
		{
			throw std::invalid_argument("Path '" + filePath.string() + "' is an invalid file");
		}

		mContext.Reset();

		std::basic_ifstream<std::uint8_t> file(filePath, std::ios::in | std::ios::binary);

		SContext& c = mContext;
		file.read(c.V.data(), c.V.size() * sizeof(std::uint8_t));
		file.read(reinterpret_cast<std::uint8_t*>(&c.I), sizeof(c.I));
		file.read(reinterpret_cast<std::uint8_t*>(&c.PC), sizeof(c.PC));
		file.read(&c.SP, sizeof(c.SP));
		file.read(&c.DT, sizeof(c.DT));
		file.read(&c.ST, sizeof(c.ST));
		file.read(reinterpret_cast<std::uint8_t*>(c.Stack.data()),
				  c.Stack.size() * sizeof(std::uint16_t));
		file.read(c.Memory.data(), c.Memory.size() * sizeof(std::uint8_t));
		file.read(c.R.data(), c.R.size() * sizeof(std::uint8_t));
		file.read(reinterpret_cast<std::uint8_t*>(&c.Display.ExtendedMode),
				  sizeof(c.Display.ExtendedMode));
		file.read(c.Display.PixelBuffer.data(),
				  c.Display.PixelBuffer.size() * sizeof(std::uint8_t));
		file.read(reinterpret_cast<std::uint8_t*>(&c.Exited), sizeof(c.Exited));

		c.DisplayChanged = true;
	}

	void CInterpreter::SaveState(const std::filesystem::path& filePath) const
	{
		if (!filePath.has_filename())
		{
			throw std::invalid_argument("Path '" + filePath.string() +
										"' is not a valid file path");
		}

		fs::path fullPath = fs::absolute(filePath);
		if (!fs::exists(fullPath.parent_path()))
		{
			throw std::invalid_argument("Parent path '" + fullPath.parent_path().string() +
										"' does not exist");
		}

		std::basic_ofstream<std::uint8_t> file(filePath, std::ios::out | std::ios::binary);

		const SContext& c = mContext;
		file.write(c.V.data(), c.V.size() * sizeof(std::uint8_t));
		file.write(reinterpret_cast<const std::uint8_t*>(&c.I), sizeof(c.I));
		file.write(reinterpret_cast<const std::uint8_t*>(&c.PC), sizeof(c.PC));
		file.write(&c.SP, sizeof(c.SP));
		file.write(&c.DT, sizeof(c.DT));
		file.write(&c.ST, sizeof(c.ST));
		file.write(reinterpret_cast<const std::uint8_t*>(c.Stack.data()),
				   c.Stack.size() * sizeof(std::uint16_t));
		file.write(c.Memory.data(), c.Memory.size() * sizeof(std::uint8_t));
		file.write(c.R.data(), c.R.size() * sizeof(std::uint8_t));
		file.write(reinterpret_cast<const std::uint8_t*>(&c.Display.ExtendedMode),
				   sizeof(c.Display.ExtendedMode));
		file.write(c.Display.PixelBuffer.data(),
				   c.Display.PixelBuffer.size() * sizeof(std::uint8_t));
		file.write(reinterpret_cast<const std::uint8_t*>(&c.Exited), sizeof(c.Exited));
	}
}
