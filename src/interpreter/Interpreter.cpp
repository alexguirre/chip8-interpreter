#include "Interpreter.h"
#include <algorithm>
#include <fstream>
#include <iterator>
#include <iostream>
#include <random>

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
	std::fill(PixelBuffer.begin(), PixelBuffer.end(), false);
	PixelBufferDirty = true;

	std::copy(CInterpreter::Fontset.begin(), CInterpreter::Fontset.end(), Memory.begin());
}

CInterpreter::CInterpreter()
	: mContext{}, mDisplay{std::make_unique<CDisplay>()}
{
}

void CInterpreter::Update()
{
	DoCycle();

	if (mContext.PixelBufferDirty)
	{
		mDisplay->UpdatePixelBuffer(mContext.PixelBuffer);
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

const SInstruction& CInterpreter::FindInstruction(std::uint16_t opcode)
{
	for (auto& inst : Instructions)
	{
		if ((opcode & inst.OpcodeMask) == inst.Opcode)
		{
			return inst;
		}
	}

	// not found, throw error
	char hexBuffer[8];
	std::snprintf(hexBuffer, std::size(hexBuffer), "%04X", opcode);
	throw std::runtime_error("Unsupported instruction '" + std::string(hexBuffer) + "'");
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

static void CLS_Handler(SContext& c)
{
	std::fill(c.PixelBuffer.begin(), c.PixelBuffer.end(), false);
	c.PixelBufferDirty = true;
}

static void RET_Handler(SContext& c)
{
	c.PC = c.Stack[--c.SP];
}

static void JP_Handler(SContext& c)
{
	c.PC = c.NNN();
}

static void CALL_Handler(SContext& c)
{
	c.Stack[c.SP++] = c.PC;
	c.PC = c.NNN();
}

static void SE_Handler(SContext& c)
{
	if (c.V[c.X()] == c.KK())
	{
		c.PC += 2;
	}
}

static void SNE_Handler(SContext& c)
{
	if (c.V[c.X()] != c.KK())
	{
		c.PC += 2;
	}
}

static void SE_2_Handler(SContext& c)
{
	if (c.V[c.X()] == c.V[c.Y()])
	{
		c.PC += 2;
	}
}

static void LD_Handler(SContext& c)
{
	c.V[c.X()] = c.KK();
}

static void ADD_Handler(SContext& c)
{
	c.V[c.X()] += c.KK();
}

static void LD_2_Handler(SContext& c)
{
	c.V[c.X()] = c.V[c.Y()];
}

static void OR_Handler(SContext& c)
{
	std::uint8_t& vx = c.V[c.X()];
	vx = vx | c.V[c.Y()];
}

static void AND_Handler(SContext& c)
{
	std::uint8_t& vx = c.V[c.X()];
	vx = vx & c.V[c.Y()];
}

static void XOR_Handler(SContext& c)
{
	std::uint8_t& vx = c.V[c.X()];
	vx = vx ^ c.V[c.Y()];
}

static void ADD_2_Handler(SContext& c)
{
	std::uint8_t& vx = c.V[c.X()];
	std::uint8_t& vy = c.V[c.Y()];
	c.V[0xF] = static_cast<std::int32_t>(vx) + static_cast<std::int32_t>(vy) > std::numeric_limits<std::uint8_t>::max();
	vx = vx + vy;
}

static void SUB_Handler(SContext& c)
{
	std::uint8_t& vx = c.V[c.X()];
	std::uint8_t& vy = c.V[c.Y()];
	c.V[0xF] =  vx > vy;
	vx = vx - vy;
}

static void SHR_Handler(SContext& c)
{
	std::uint8_t& vx = c.V[c.X()];
	c.V[0xF] = vx & 1;
	vx >>= 1;
}

static void SUBN_Handler(SContext& c)
{
	std::uint8_t& vx = c.V[c.X()];
	std::uint8_t& vy = c.V[c.Y()];
	c.V[0xF] = vy > vx;
	vx = vy - vx;
}

static void SHL_Handler(SContext& c)
{
	std::uint8_t& vx = c.V[c.X()];
	c.V[0xF] = vx & 0x80;
	vx <<= 1;
}

static void SNE_2_Handler(SContext& c)
{
	if (c.V[c.X()] != c.V[c.Y()])
	{
		c.PC += 2;
	}
}

static void LD_I_Handler(SContext& c)
{
	c.I = c.IR & c.NNN();
}

static void JP_2_Handler(SContext& c)
{
	c.PC = c.NNN() + c.V[0];
}

static void RND_Handler(SContext& c)
{
	static std::mt19937 gen{ std::random_device{}() };
	static std::uniform_int_distribution<std::int32_t> dist{
		std::numeric_limits<std::uint8_t>::min(), std::numeric_limits<std::uint8_t>::max()
	};

	std::uint8_t rnd = static_cast<std::uint8_t>(dist(gen));
	c.V[c.X()] = rnd & c.KK();
}

static void DRW_Handler(SContext& c)
{
	std::uint8_t& vx = c.V[c.X()];
	std::uint8_t& vy = c.V[c.Y()];
	std::uint8_t n = c.N();

	c.V[0xF] = 0; // collision flag to 0
	for (std::uint8_t byteIndex = 0; byteIndex < n; byteIndex++)
	{
		std::uint8_t byte = c.Memory[c.I + byteIndex];

		for (std::uint8_t bitIndex = 0; bitIndex < 8; bitIndex++)
		{
			std::uint8_t bit = (byte >> bitIndex) & 1;
			std::uint8_t x = (vx + (7 - bitIndex)) % std::get<0>(CDisplay::Resolution);
			std::uint8_t y = (vy + byteIndex) % std::get<1>(CDisplay::Resolution);
			std::uint8_t& pixel = c.PixelBuffer[x + y * std::get<0>(CDisplay::Resolution)];

			// collision
			if (bit && pixel)
			{
				c.V[0xF] = 1;
			}

			pixel ^= bit;
		}
	}
}

static void SKP_Handler(SContext&)
{
	// TODO: SKP, requires keyboard input
}

static void SKNP_Handler(SContext&)
{
	// TODO: SKNP, requires keyboard input
}

static void LD_3_Handler(SContext& c)
{
	c.V[c.X()] = c.DT;
}

static void LD_4_Handler(SContext&)
{
	// TODO: LD Vx, K, requires keyboard input
}

static void LD_5_Handler(SContext& c)
{
	c.DT = c.V[c.X()];
}

static void LD_6_Handler(SContext& c)
{
	c.ST = c.V[c.X()];
}

static void ADD_I_Handler(SContext& c)
{
	c.I += c.V[c.X()];
}

static void LD_7_Handler(SContext& c)
{
	c.I = CInterpreter::FontsetCharByteSize * c.V[c.X()];
}

static void LD_8_Handler(SContext& c)
{
	std::uint8_t& vx = c.V[c.X()];
	std::uint8_t ones = vx % 10;
	std::uint8_t tens = (vx / 10) % 10;
	std::uint8_t hundreds = (vx / 100) % 10;

	c.Memory[c.I + 0] = hundreds;
	c.Memory[c.I + 1] = tens;
	c.Memory[c.I + 2] = ones;
}

static void LD_9_Handler(SContext& c)
{
	std::uint8_t x = c.X();
	for (std::uint8_t i = 0; i <= x; i++)
	{
		c.Memory[c.I + i] = c.V[i];
	}
}

static void LD_10_Handler(SContext& c)
{
	std::uint8_t x = c.X();
	for (std::uint8_t i = 0; i <= x; i++)
	{
		c.V[i] = c.Memory[c.I + i];
	}
}

const std::vector<SInstruction> CInterpreter::Instructions =
{
	{ "CLS",	CLS_Handler,	0x00E0,	0xF0FF },
	{ "RET",	RET_Handler,	0x00EE,	0xF0FF },
	{ "JP",		JP_Handler,		0x1000,	0xF000 },
	{ "CALL",	CALL_Handler,	0x2000,	0xF000 },
	{ "SE",		SE_Handler,		0x3000,	0xF000 },
	{ "SNE",	SNE_Handler,	0x4000,	0xF000 },
	{ "SE",		SE_2_Handler,	0x5000,	0xF00F },
	{ "LD",		LD_Handler,		0x6000,	0xF000 },
	{ "ADD",	ADD_Handler,	0x7000,	0xF000 },
	{ "LD",		LD_2_Handler,	0x8000,	0xF00F },
	{ "OR",		OR_Handler,		0x8001,	0xF00F },
	{ "AND",	AND_Handler,	0x8002,	0xF00F },
	{ "XOR",	XOR_Handler,	0x8003,	0xF00F },
	{ "ADD",	ADD_2_Handler,	0x8004,	0xF00F },
	{ "SUB",	SUB_Handler,	0x8005,	0xF00F },
	{ "SHR",	SHR_Handler,	0x8006,	0xF00F },
	{ "SUBN",	SUBN_Handler,	0x8007,	0xF00F },
	{ "SHL",	SHL_Handler,	0x800E,	0xF00F },
	{ "SNE",	SNE_2_Handler,	0x9000,	0xF00F },
	{ "LD",		LD_I_Handler,	0xA000,	0xF000 },
	{ "JP",		JP_2_Handler,	0xB000,	0xF000 },
	{ "RND",	RND_Handler,	0xC000,	0xF000 },
	{ "DRW",	DRW_Handler,	0xD000,	0xF000 },
	{ "SKP",	SKP_Handler,	0xE09E,	0xF0FF },
	{ "SKNP",	SKNP_Handler,	0xE0A1,	0xF0FF },
	{ "LD",		LD_3_Handler,	0xF007,	0xF0FF },
	{ "LD",		LD_4_Handler,	0xF00A,	0xF0FF },
	{ "LD",		LD_5_Handler,	0xF015,	0xF0FF },
	{ "LD",		LD_6_Handler,	0xF018,	0xF0FF },
	{ "ADD",	ADD_I_Handler,	0xF01E,	0xF0FF },
	{ "LD",		LD_7_Handler,	0xF029,	0xF0FF },
	{ "LD",		LD_8_Handler,	0xF033,	0xF0FF },
	{ "LD",		LD_9_Handler,	0xF055,	0xF0FF },
	{ "LD",		LD_10_Handler,	0xF065,	0xF0FF },
};
