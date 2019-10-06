#include "Instructions.h"
#include <sstream>
#include <random>
#include <iomanip>
#include <gsl/gsl_util>
#include "Interpreter.h"

using namespace c8;
using namespace c8::constants;

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

static void Handler_CLS(SContext& c)
{
	std::fill(c.PixelBuffer.begin(), c.PixelBuffer.end(), std::uint8_t(0));
	c.PixelBufferDirty = true;
}

static void Handler_RET(SContext& c)
{
	c.PC = c.Stack[--c.SP];
}

static void Handler_JP_nnn(SContext& c)
{
	c.PC = c.NNN();
}

static void Handler_CALL_nnn(SContext& c)
{
	c.Stack[c.SP++] = c.PC;
	c.PC = c.NNN();
}

static void Handler_SE_Vx_kk(SContext& c)
{
	if (c.V[c.X()] == c.KK())
	{
		c.PC += InstructionByteSize;
	}
}

static void Handler_SNE_Vx_kk(SContext& c)
{
	if (c.V[c.X()] != c.KK())
	{
		c.PC += InstructionByteSize;
	}
}

static void Handler_SE_Vx_Vy(SContext& c)
{
	if (c.V[c.X()] == c.V[c.Y()])
	{
		c.PC += InstructionByteSize;
	}
}

static void Handler_LD_Vx_kk(SContext& c)
{
	c.V[c.X()] = c.KK();
}

static void Handler_ADD_Vx_kk(SContext& c)
{
	c.V[c.X()] += c.KK();
}

static void Handler_LD_Vx_Vy(SContext& c)
{
	c.V[c.X()] = c.V[c.Y()];
}

static void Handler_OR_Vx_Vy(SContext& c)
{
	c.V[c.X()] |= c.V[c.Y()];
}

static void Handler_AND_Vx_Vy(SContext& c)
{
	c.V[c.X()] &= c.V[c.Y()];
}

static void Handler_XOR_Vx_Vy(SContext& c)
{
	c.V[c.X()] ^= c.V[c.Y()];
}

static void Handler_ADD_Vx_Vy(SContext& c)
{
	std::uint8_t& vx = c.V[c.X()];
	const std::uint8_t vy = c.V[c.Y()];
	c.V[0xF] = (vx + vy) > std::numeric_limits<std::uint8_t>::max();
	vx += vy;
}

static void Handler_SUB_Vx_Vy(SContext& c)
{
	std::uint8_t& vx = c.V[c.X()];
	const std::uint8_t vy = c.V[c.Y()];
	c.V[0xF] = vx > vy;
	vx -= vy;
}

static void Handler_SHR_Vx(SContext& c)
{
	std::uint8_t& vx = c.V[c.X()];
	c.V[0xF] = vx & 1;
	vx >>= 1;
}

static void Handler_SUBN_Vx_Vy(SContext& c)
{
	std::uint8_t& vx = c.V[c.X()];
	const std::uint8_t vy = c.V[c.Y()];
	c.V[0xF] = vy > vx;
	vx = vy - vx;
}

static void Handler_SHL_Vx(SContext& c)
{
	std::uint8_t& vx = c.V[c.X()];
	c.V[0xF] = (vx >> 7) & 1;
	vx <<= 1;
}

static void Handler_SNE_Vx_Vy(SContext& c)
{
	if (c.V[c.X()] != c.V[c.Y()])
	{
		c.PC += InstructionByteSize;
	}
}

static void Handler_LD_I_nnn(SContext& c)
{
	c.I = c.NNN();
}

static void Handler_JP_V0_nnn(SContext& c)
{
	c.PC = c.NNN() + c.V[0];
}

static void Handler_RND_Vx_kk(SContext& c)
{
	static std::mt19937 gen{ std::random_device{}() };
	static std::uniform_int_distribution<std::int32_t> dist{
		std::numeric_limits<std::uint8_t>::min(), std::numeric_limits<std::uint8_t>::max()
	};

	const std::uint8_t rnd = gsl::narrow<std::uint8_t>(dist(gen));
	c.V[c.X()] = rnd & c.KK();
}

static void Handler_DRW_Vx_Vy_n(SContext& c)
{
	const std::uint8_t vx = c.V[c.X()];
	const std::uint8_t vy = c.V[c.Y()];
	const std::uint8_t n = c.N();

	c.V[0xF] = 0; // collision flag to 0
	for (std::size_t byteIndex = 0; byteIndex < n; byteIndex++)
	{
		const std::uint8_t byte = c.Memory[c.I + byteIndex];

		for (std::size_t bitIndex = 0; bitIndex < 8; bitIndex++)
		{
			const std::uint8_t bit = (byte >> (7 - bitIndex)) & 1;
			const std::size_t x = (vx + bitIndex) % DisplayResolutionWidth;
			const std::size_t y = (vy + byteIndex) % DisplayResolutionHeight;
			std::uint8_t& pixel = c.PixelBuffer[x + y * DisplayResolutionWidth];

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

static void Handler_SKP_Vx(SContext& c)
{
	const std::uint8_t vx = c.V[c.X()];
	if (c.Keyboard[vx])
	{
		c.PC += InstructionByteSize;
	}
}

static void Handler_SKNP_Vx(SContext& c)
{
	const std::uint8_t vx = c.V[c.X()];
	if (!c.Keyboard[vx])
	{
		c.PC += InstructionByteSize;
	}
}

static void Handler_LD_Vx_DT(SContext& c)
{
	c.V[c.X()] = c.DT;
}

static void Handler_LD_Vx_K(SContext& c)
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
	c.PC -= InstructionByteSize;
}

static void Handler_LD_DT_Vx(SContext& c)
{
	c.DT = c.V[c.X()];
}

static void Handler_LD_ST_Vx(SContext& c)
{
	c.ST = c.V[c.X()];
}

static void Handler_ADD_I_Vx(SContext& c)
{
	const std::uint8_t vx = c.V[c.X()];
	c.V[0xF] = (vx + c.I) > std::numeric_limits<std::uint8_t>::max();
	c.I += vx;
}

static void Handler_LD_F_Vx(SContext& c)
{
	c.I = FontsetCharByteSize * c.V[c.X()];
}

static void Handler_LD_B_Vx(SContext& c)
{
	const std::uint8_t vx = c.V[c.X()];
	const std::uint8_t ones = vx % 10;
	const std::uint8_t tens = (vx / 10) % 10;
	const std::uint8_t hundreds = (vx / 100) % 10;

	c.Memory[c.I + std::size_t{ 0 }] = hundreds;
	c.Memory[c.I + std::size_t{ 1 }] = tens;
	c.Memory[c.I + std::size_t{ 2 }] = ones;
}

static void Handler_LD_derefI_Vx(SContext& c)
{
	const std::uint8_t x = c.X();
	for (std::size_t i = 0; i <= x; i++)
	{
		c.Memory[c.I + i] = c.V[i];
	}
	c.I += x + 1;
}

static void Handler_LD_Vx_derefI(SContext& c)
{
	const std::uint8_t x = c.X();
	for (std::size_t i = 0; i <= x; i++)
	{
		c.V[i] = c.Memory[c.I + i];
	}
	c.I += x + 1;
}

using namespace std::placeholders;

namespace c8
{
	const std::vector<SInstruction> SInstruction::InstructionSet =
	{
		{ "CLS",	Handler_CLS,			0x00E0,	0xF0FF,	ToString_NAME									},
		{ "RET",	Handler_RET,			0x00EE,	0xF0FF,	ToString_NAME									},
		{ "JP",		Handler_JP_nnn,			0x1000,	0xF000,	ToString_NAME_nnn								},
		{ "CALL",	Handler_CALL_nnn,		0x2000,	0xF000,	ToString_NAME_nnn								},
		{ "SE",		Handler_SE_Vx_kk,		0x3000,	0xF000,	ToString_NAME_Vx_kk								},
		{ "SNE",	Handler_SNE_Vx_kk,		0x4000,	0xF000,	ToString_NAME_Vx_kk								},
		{ "SE",		Handler_SE_Vx_Vy,		0x5000,	0xF00F,	ToString_NAME_Vx_Vy								},
		{ "LD",		Handler_LD_Vx_kk,		0x6000,	0xF000,	ToString_NAME_Vx_kk								},
		{ "ADD",	Handler_ADD_Vx_kk,		0x7000,	0xF000,	ToString_NAME_Vx_kk								},
		{ "LD",		Handler_LD_Vx_Vy,		0x8000,	0xF00F,	ToString_NAME_Vx_Vy								},
		{ "OR",		Handler_OR_Vx_Vy,		0x8001,	0xF00F,	ToString_NAME_Vx_Vy								},
		{ "AND",	Handler_AND_Vx_Vy,		0x8002,	0xF00F,	ToString_NAME_Vx_Vy								},
		{ "XOR",	Handler_XOR_Vx_Vy,		0x8003,	0xF00F,	ToString_NAME_Vx_Vy								},
		{ "ADD",	Handler_ADD_Vx_Vy,		0x8004,	0xF00F,	ToString_NAME_Vx_Vy								},
		{ "SUB",	Handler_SUB_Vx_Vy,		0x8005,	0xF00F,	ToString_NAME_Vx_Vy								},
		{ "SHR",	Handler_SHR_Vx,			0x8006,	0xF00F,	ToString_NAME_Vx								},
		{ "SUBN",	Handler_SUBN_Vx_Vy,		0x8007,	0xF00F,	ToString_NAME_Vx_Vy								},
		{ "SHL",	Handler_SHL_Vx,			0x800E,	0xF00F,	ToString_NAME_Vx								},
		{ "SNE",	Handler_SNE_Vx_Vy,		0x9000,	0xF00F,	ToString_NAME_Vx_Vy								},
		{ "LD",		Handler_LD_I_nnn,		0xA000,	0xF000,	std::bind(ToString_NAME_dst_nnn, _1, _2, "I")	},
		{ "JP",		Handler_JP_V0_nnn,		0xB000,	0xF000,	std::bind(ToString_NAME_dst_nnn, _1, _2, "V0")	},
		{ "RND",	Handler_RND_Vx_kk,		0xC000,	0xF000,	ToString_NAME_Vx_kk								},
		{ "DRW",	Handler_DRW_Vx_Vy_n,	0xD000,	0xF000,	ToString_NAME_Vx_Vy_n							},
		{ "SKP",	Handler_SKP_Vx,			0xE09E,	0xF0FF,	ToString_NAME_Vx								},
		{ "SKNP",	Handler_SKNP_Vx,		0xE0A1,	0xF0FF,	ToString_NAME_Vx								},
		{ "LD",		Handler_LD_Vx_DT,		0xF007,	0xF0FF,	std::bind(ToString_NAME_Vx_src, _1, _2, "DT")	},
		{ "LD",		Handler_LD_Vx_K,		0xF00A,	0xF0FF,	std::bind(ToString_NAME_Vx_src, _1, _2, "K")	},
		{ "LD",		Handler_LD_DT_Vx,		0xF015,	0xF0FF,	std::bind(ToString_NAME_dst_Vx, _1, _2, "DT")	},
		{ "LD",		Handler_LD_ST_Vx,		0xF018,	0xF0FF,	std::bind(ToString_NAME_dst_Vx, _1, _2, "ST")	},
		{ "ADD",	Handler_ADD_I_Vx,		0xF01E,	0xF0FF,	std::bind(ToString_NAME_dst_Vx, _1, _2, "I")	},
		{ "LD",		Handler_LD_F_Vx,		0xF029,	0xF0FF,	std::bind(ToString_NAME_dst_Vx, _1, _2, "F")	},
		{ "LD",		Handler_LD_B_Vx,		0xF033,	0xF0FF,	std::bind(ToString_NAME_dst_Vx, _1, _2, "B")	},
		{ "LD",		Handler_LD_derefI_Vx,	0xF055,	0xF0FF,	std::bind(ToString_NAME_dst_Vx, _1, _2, "[I]")	},
		{ "LD",		Handler_LD_Vx_derefI,	0xF065,	0xF0FF,	std::bind(ToString_NAME_Vx_src, _1, _2, "[I]")	},
	};
}
