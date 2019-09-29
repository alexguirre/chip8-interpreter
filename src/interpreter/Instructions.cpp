#include "Instructions.h"
#include <sstream>
#include <random>
#include "Interpreter.h"

static std::string ToString_NAME(const SInstruction& i, const SContext&)
{
	return i.Name;
}

static std::string ToString_NAME_nnn(const SInstruction& i, const SContext& c)
{
	std::ostringstream ss;
	ss << i.Name << " " << std::uppercase << std::hex << std::setfill('0') << std::setw(3) << static_cast<std::uint32_t>(c.NNN());
	return ss.str();
}

static std::string ToString_NAME_Vx_kk(const SInstruction& i, const SContext& c)
{
	std::ostringstream ss;
	ss << i.Name << " V" << std::uppercase << std::hex << static_cast<std::uint32_t>(c.X()) << ", " << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << static_cast<std::uint32_t>(c.KK());
	return ss.str();
}

static std::string ToString_NAME_Vx_Vy(const SInstruction& i, const SContext& c)
{
	std::ostringstream ss;
	ss << i.Name << " V" << std::uppercase << std::hex << static_cast<std::uint32_t>(c.X()) << ", V" << std::uppercase << std::hex << static_cast<std::uint32_t>(c.Y());
	return ss.str();
}

static std::string ToString_NAME_Vx(const SInstruction& i, const SContext& c)
{
	std::ostringstream ss;
	ss << i.Name << " V" << std::uppercase << std::uppercase << std::hex << static_cast<std::uint32_t>(c.X());
	return ss.str();
}

static std::string ToString_NAME_dst_nnn(const SInstruction& i, const SContext& c, std::string_view dstName)
{
	std::ostringstream ss;
	ss << i.Name << " " << dstName <<", " << std::uppercase << std::hex << std::setfill('0') << std::setw(3) << static_cast<std::uint32_t>(c.NNN());
	return ss.str();
}

static std::string ToString_NAME_Vx_Vy_n(const SInstruction& i, const SContext& c)
{
	std::ostringstream ss;
	ss << i.Name << " V" << std::uppercase << std::hex << static_cast<std::uint32_t>(c.X()) << ", V" << std::uppercase << std::hex << static_cast<std::uint32_t>(c.Y())
		<< ", " << std::uppercase << std::hex << static_cast<std::uint32_t>(c.N());
	return ss.str();
}

static std::string ToString_NAME_Vx_src(const SInstruction& i, const SContext& c, std::string_view srcName)
{
	std::ostringstream ss;
	ss << i.Name << " V" << std::uppercase << std::uppercase << std::hex << static_cast<std::uint32_t>(c.X()) << ", " << srcName;
	return ss.str();
}

static std::string ToString_NAME_dst_Vx(const SInstruction& i, const SContext& c, std::string_view dstName)
{
	std::ostringstream ss;
	ss << i.Name << " " << dstName << ", V" << std::uppercase << std::uppercase << std::hex << static_cast<std::uint32_t>(c.X());
	return ss.str();
}

static void CLS_Handler(SContext& c)
{
	std::fill(c.PixelBuffer.begin(), c.PixelBuffer.end(), std::uint8_t(0));
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
		c.PC += CInterpreter::InstructionByteSize;
	}
}

static void SNE_Handler(SContext& c)
{
	if (c.V[c.X()] != c.KK())
	{
		c.PC += CInterpreter::InstructionByteSize;
	}
}

static void SE_2_Handler(SContext& c)
{
	if (c.V[c.X()] == c.V[c.Y()])
	{
		c.PC += CInterpreter::InstructionByteSize;
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
	c.V[c.X()] |= c.V[c.Y()];
}

static void AND_Handler(SContext& c)
{
	c.V[c.X()] &= c.V[c.Y()];
}

static void XOR_Handler(SContext& c)
{
	c.V[c.X()] ^= c.V[c.Y()];
}

static void ADD_2_Handler(SContext& c)
{
	std::uint8_t& vx = c.V[c.X()];
	std::uint8_t& vy = c.V[c.Y()];
	c.V[0xF] = static_cast<std::int32_t>(vx) + static_cast<std::int32_t>(vy) > std::numeric_limits<std::uint8_t>::max();
	vx += vy;
}

static void SUB_Handler(SContext& c)
{
	std::uint8_t& vx = c.V[c.X()];
	std::uint8_t& vy = c.V[c.Y()];
	c.V[0xF] = vx > vy;
	vx -= vy;
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
	c.V[0xF] = (vx >> 7) & 1;
	vx <<= 1;
}

static void SNE_2_Handler(SContext& c)
{
	if (c.V[c.X()] != c.V[c.Y()])
	{
		c.PC += CInterpreter::InstructionByteSize;
	}
}

static void LD_I_Handler(SContext& c)
{
	c.I = c.NNN();
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
	std::uint8_t vx = c.V[c.X()];
	std::uint8_t vy = c.V[c.Y()];
	std::uint8_t n = c.N();

	c.V[0xF] = 0; // collision flag to 0
	for (std::uint8_t byteIndex = 0; byteIndex < n; byteIndex++)
	{
		std::uint8_t byte = c.Memory[c.I + byteIndex];

		for (std::uint8_t bitIndex = 0; bitIndex < 8; bitIndex++)
		{
			std::uint8_t bit = (byte >> (7 - bitIndex)) & 1;
			std::size_t x = (vx + bitIndex) % CDisplay::ResolutionWidth;
			std::size_t y = (vy + byteIndex) % CDisplay::ResolutionHeight;
			std::uint8_t& pixel = c.PixelBuffer[x + y * CDisplay::ResolutionWidth];

			// collision
			if (bit && pixel)
			{
				c.V[0xF] = 1;
			}

			pixel ^= bit;
		}
	}
	c.PixelBufferDirty = true;
}

static void SKP_Handler(SContext& c)
{
	std::uint8_t vx = c.V[c.X()];
	if (c.Keyboard[vx])
	{
		c.PC += CInterpreter::InstructionByteSize;
	}
}

static void SKNP_Handler(SContext& c)
{
	std::uint8_t vx = c.V[c.X()];
	if (!c.Keyboard[vx])
	{
		c.PC += CInterpreter::InstructionByteSize;
	}
}

static void LD_3_Handler(SContext& c)
{
	c.V[c.X()] = c.DT;
}

static void LD_4_Handler(SContext& c)
{
	for (std::uint8_t key = 0; key < c.Keyboard.size(); key++)
	{
		if (c.Keyboard[key])
		{
			c.V[c.X()] = key;
			return;
		}
	}

	// no key was pressed, move the PC back to execute this instruction again in the next cycle
	c.PC -= CInterpreter::InstructionByteSize;
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
	std::uint8_t& vx = c.V[c.X()];
	c.V[0xF] = static_cast<std::int32_t>(vx) + static_cast<std::int32_t>(c.I) > std::numeric_limits<std::uint8_t>::max();
	c.I += vx;
}

static void LD_7_Handler(SContext& c)
{
	c.I = CInterpreter::FontsetCharByteSize * c.V[c.X()];
}

static void LD_8_Handler(SContext& c)
{
	std::uint8_t vx = c.V[c.X()];
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
	c.I += x + 1;
}

static void LD_10_Handler(SContext& c)
{
	std::uint8_t x = c.X();
	for (std::uint8_t i = 0; i <= x; i++)
	{
		c.V[i] = c.Memory[c.I + i];
	}
	c.I += x + 1;
}

using namespace std::placeholders;

const std::vector<SInstruction> SInstruction::InstructionSet =
{
	{ "CLS",	CLS_Handler,	0x00E0,	0xF0FF,	ToString_NAME									},
	{ "RET",	RET_Handler,	0x00EE,	0xF0FF,	ToString_NAME									},
	{ "JP",		JP_Handler,		0x1000,	0xF000,	ToString_NAME_nnn								},
	{ "CALL",	CALL_Handler,	0x2000,	0xF000,	ToString_NAME_nnn								},
	{ "SE",		SE_Handler,		0x3000,	0xF000,	ToString_NAME_Vx_kk								},
	{ "SNE",	SNE_Handler,	0x4000,	0xF000,	ToString_NAME_Vx_kk								},
	{ "SE",		SE_2_Handler,	0x5000,	0xF00F,	ToString_NAME_Vx_Vy								},
	{ "LD",		LD_Handler,		0x6000,	0xF000,	ToString_NAME_Vx_kk								},
	{ "ADD",	ADD_Handler,	0x7000,	0xF000,	ToString_NAME_Vx_kk								},
	{ "LD",		LD_2_Handler,	0x8000,	0xF00F,	ToString_NAME_Vx_Vy								},
	{ "OR",		OR_Handler,		0x8001,	0xF00F,	ToString_NAME_Vx_Vy								},
	{ "AND",	AND_Handler,	0x8002,	0xF00F,	ToString_NAME_Vx_Vy								},
	{ "XOR",	XOR_Handler,	0x8003,	0xF00F,	ToString_NAME_Vx_Vy								},
	{ "ADD",	ADD_2_Handler,	0x8004,	0xF00F,	ToString_NAME_Vx_Vy								},
	{ "SUB",	SUB_Handler,	0x8005,	0xF00F,	ToString_NAME_Vx_Vy								},
	{ "SHR",	SHR_Handler,	0x8006,	0xF00F,	ToString_NAME_Vx								},
	{ "SUBN",	SUBN_Handler,	0x8007,	0xF00F,	ToString_NAME_Vx_Vy								},
	{ "SHL",	SHL_Handler,	0x800E,	0xF00F,	ToString_NAME_Vx								},
	{ "SNE",	SNE_2_Handler,	0x9000,	0xF00F,	ToString_NAME_Vx_Vy								},
	{ "LD",		LD_I_Handler,	0xA000,	0xF000,	std::bind(ToString_NAME_dst_nnn, _1, _2, "I")	},
	{ "JP",		JP_2_Handler,	0xB000,	0xF000,	std::bind(ToString_NAME_dst_nnn, _1, _2, "V0")	},
	{ "RND",	RND_Handler,	0xC000,	0xF000,	ToString_NAME_Vx_kk								},
	{ "DRW",	DRW_Handler,	0xD000,	0xF000,	ToString_NAME_Vx_Vy_n							},
	{ "SKP",	SKP_Handler,	0xE09E,	0xF0FF,	ToString_NAME_Vx								},
	{ "SKNP",	SKNP_Handler,	0xE0A1,	0xF0FF,	ToString_NAME_Vx								},
	{ "LD",		LD_3_Handler,	0xF007,	0xF0FF,	std::bind(ToString_NAME_Vx_src, _1, _2, "DT")	},
	{ "LD",		LD_4_Handler,	0xF00A,	0xF0FF,	std::bind(ToString_NAME_Vx_src, _1, _2, "K")	},
	{ "LD",		LD_5_Handler,	0xF015,	0xF0FF,	std::bind(ToString_NAME_dst_Vx, _1, _2, "DT")	},
	{ "LD",		LD_6_Handler,	0xF018,	0xF0FF,	std::bind(ToString_NAME_dst_Vx, _1, _2, "ST")	},
	{ "ADD",	ADD_I_Handler,	0xF01E,	0xF0FF,	std::bind(ToString_NAME_dst_Vx, _1, _2, "I")	},
	{ "LD",		LD_7_Handler,	0xF029,	0xF0FF,	std::bind(ToString_NAME_dst_Vx, _1, _2, "F")	},
	{ "LD",		LD_8_Handler,	0xF033,	0xF0FF,	std::bind(ToString_NAME_dst_Vx, _1, _2, "B")	},
	{ "LD",		LD_9_Handler,	0xF055,	0xF0FF,	std::bind(ToString_NAME_dst_Vx, _1, _2, "[I]")	},
	{ "LD",		LD_10_Handler,	0xF065,	0xF0FF,	std::bind(ToString_NAME_Vx_src, _1, _2, "[I]")	},
};
