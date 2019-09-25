#include "Interpreter.h"
#include <algorithm>
#include <fstream>
#include <iterator>
#include <iostream>
#include <random>
#include <sstream>

namespace fs = std::filesystem;

SContext::SContext()
{
	Reset();
}

void SContext::Reset()
{
	std::fill(V.begin(), V.end(), std::uint8_t(0));
	I = 0;
	PC = 0;
	SP = 0;
	DT = 0;
	ST = 0;
	IR = 0;
	std::fill(Stack.begin(), Stack.end(), std::uint16_t(0));
	std::fill(Memory.begin(), Memory.end(), std::uint8_t(0));
	std::fill(PixelBuffer.begin(), PixelBuffer.end(), std::uint8_t(0));
	PixelBufferDirty = true;
	std::fill(Keyboard.begin(), Keyboard.end(), false);

	std::copy(CInterpreter::Fontset.begin(), CInterpreter::Fontset.end(), Memory.begin());
}

CInterpreter::CInterpreter()
	: mContext{}, mDisplay{ std::make_unique<CDisplay>() },
		mKeyboard{ std::make_unique<CKeyboard>() }, mSound{ std::make_unique<CSound>() }
{
}

void CInterpreter::Update()
{
	mKeyboard->GetState(mContext.Keyboard);

	DoCycle();

	auto clockNow = std::chrono::high_resolution_clock::now();
	if (std::chrono::duration_cast<std::chrono::milliseconds>(clockNow - mClockPrev).count() >= ClockRateMs)
	{
		DoTick();
		mClockPrev = clockNow;
	}

	mDisplay->Update();
}

void CInterpreter::DoCycle()
{
	SContext& c = mContext;

	// fetch
	std::uint16_t opcode = c.Memory[c.PC] << 8 | c.Memory[c.PC + 1];
	c.IR = opcode;

	// execute
	const SInstruction& instr = FindInstruction(opcode);
	instr.Handler(c);

	// move to next instruction
	c.PC += 2;
}

void CInterpreter::DoTick()
{
	if (mContext.DT > 0)
	{
		mContext.DT--;
	}

	if (mContext.ST > 0)
	{
		mContext.ST--;
		if (mContext.ST == 0)
		{
			using namespace std::chrono_literals;
			mSound->Beep(550, 50ms);
		}
	}

	if (mContext.PixelBufferDirty)
	{
		mDisplay->UpdatePixelBuffer(mContext.PixelBuffer);
	}
}

void CInterpreter::LoadProgram(const std::filesystem::path& filePath)
{
	if (!fs::is_regular_file(filePath))
	{
		throw std::invalid_argument("Path '" + filePath.string() + "' is an invalid file");
	}

	mContext.Reset();

	std::basic_ifstream<std::uint8_t> file(filePath, std::ios::in | std::ios::binary);
	std::copy(
		std::istreambuf_iterator(file),
		std::istreambuf_iterator<std::uint8_t>(),
		std::next(mContext.Memory.begin(), ProgramStartAddress)
	);

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
	file.read(reinterpret_cast<std::uint8_t*>(c.Stack.data()), c.Stack.size() * sizeof(std::uint16_t));
	file.read(c.Memory.data(), c.Memory.size() * sizeof(std::uint8_t));
	file.read(c.PixelBuffer.data(), c.PixelBuffer.size() * sizeof(std::uint8_t));
	
	c.PixelBufferDirty = true;
}

void CInterpreter::SaveState(const std::filesystem::path& filePath) const
{
	if (!filePath.has_filename())
	{
		throw std::invalid_argument("Path '" + filePath.string() + "' is not a valid file path");
	}

	fs::path fullPath = fs::absolute(filePath);
	if (!fs::exists(fullPath.parent_path()))
	{
		throw std::invalid_argument("Parent path '" + fullPath.parent_path().string() + "' does not exist");
	}

	std::basic_ofstream<std::uint8_t> file(filePath, std::ios::out | std::ios::binary);

	const SContext& c = mContext;
	file.write(c.V.data(), c.V.size() * sizeof(std::uint8_t));
	file.write(reinterpret_cast<const std::uint8_t*>(&c.I), sizeof(c.I));
	file.write(reinterpret_cast<const std::uint8_t*>(&c.PC), sizeof(c.PC));
	file.write(&c.SP, sizeof(c.SP));
	file.write(&c.DT, sizeof(c.DT));
	file.write(&c.ST, sizeof(c.ST));
	file.write(reinterpret_cast<const std::uint8_t*>(c.Stack.data()), c.Stack.size() * sizeof(std::uint16_t));
	file.write(c.Memory.data(), c.Memory.size() * sizeof(std::uint8_t));
	file.write(c.PixelBuffer.data(), c.PixelBuffer.size() * sizeof(std::uint8_t));
}

const SInstruction& CInterpreter::FindInstruction(std::uint16_t opcode) const
{
	auto inst = TryFindInstruction(opcode);

	if (!inst.has_value())
	{
		// not found, throw error
		char hexBuffer[8];
		std::snprintf(hexBuffer, std::size(hexBuffer), "%04X", opcode);
		throw std::runtime_error("Unsupported instruction '" + std::string(hexBuffer) + "'");
	}

	return inst.value();
}

std::optional<std::reference_wrapper<const SInstruction>> CInterpreter::TryFindInstruction(std::uint16_t opcode) const
{
	for (auto& inst : SInstruction::InstructionSet)
	{
		if ((opcode & inst.OpcodeMask) == inst.Opcode)
		{
			return std::cref(inst);
		}
	}

	return std::nullopt;
}

const std::array<std::uint8_t, 16 * CInterpreter::FontsetCharByteSize> CInterpreter::Fontset
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
