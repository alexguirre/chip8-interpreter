#include "Instructions.h"
#include <sstream>
#include <random>
#include <iomanip>
#include <gsl/gsl_util>
#include "Interpreter.h"
#include <doctest/doctest.h>

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
	Expects(c.SP > 0 && c.SP <= c.Stack.size());

	c.PC = c.Stack[--c.SP];
}

static void Handler_JP_nnn(SContext& c)
{
	c.PC = c.NNN();
}

static void Handler_CALL_nnn(SContext& c)
{
	Expects(c.SP >= 0 && c.SP < c.Stack.size());

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
}

static void Handler_LD_Vx_derefI(SContext& c)
{
	const std::uint8_t x = c.X();
	for (std::size_t i = 0; i <= x; i++)
	{
		c.V[i] = c.Memory[c.I + i];
	}
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

TEST_CASE("Instruction ToString")
{
	SInstruction i{ "A", [](SContext&) {}, 0x0000, 0x0000, [](const SInstruction&, const SContext&) { return std::string{}; } };
	SContext c{};

	SUBCASE("NAME")
	{		
		CHECK(ToString_NAME(i, c) == "A");
	}

	SUBCASE("NAME nnn")
	{
		c.IR = 0x0012;
		CHECK(ToString_NAME_nnn(i, c) == "A 012");
	}

	SUBCASE("NAME Vx, kk")
	{
		c.IR = 0x0123;
		CHECK(ToString_NAME_Vx_kk(i, c) == "A V1, 23");
	}

	SUBCASE("NAME Vx, Vy")
	{
		c.IR = 0x0120;
		CHECK(ToString_NAME_Vx_Vy(i, c) == "A V1, V2");
	}

	SUBCASE("NAME Vx")
	{
		c.IR = 0x0100;
		CHECK(ToString_NAME_Vx(i, c) == "A V1");
	}

	SUBCASE("NAME dst, nnn")
	{
		c.IR = 0x0012;
		CHECK(ToString_NAME_dst_nnn(i, c, "D") == "A D, 012");
	}

	SUBCASE("NAME Vx, Vy, n")
	{
		c.IR = 0x0123;
		CHECK(ToString_NAME_Vx_Vy_n(i, c) == "A V1, V2, 3");
	}

	SUBCASE("NAME Vx, src")
	{
		c.IR = 0x0100;
		CHECK(ToString_NAME_Vx_src(i, c, "S") == "A V1, S");
	}

	SUBCASE("NAME dst, Vx")
	{
		c.IR = 0x0100;
		CHECK(ToString_NAME_dst_Vx(i, c, "D") == "A D, V1");
	}
}

TEST_CASE("Instruction: CLS")
{
	SContext c{};
	std::fill(c.PixelBuffer.begin(), c.PixelBuffer.end(), std::uint8_t{ 1 });

	Handler_CLS(c);
	
	CHECK(c.PixelBufferDirty);
	CHECK(std::all_of(c.PixelBuffer.begin(), c.PixelBuffer.end(), [](std::uint8_t b) { return b == 0; }));
}

TEST_CASE("Instruction: RET")
{
	SContext c{};
	c.Stack[0] = 0x1111;
	c.Stack[1] = 0x2222;
	c.SP = 2;
	c.PC = 0;

	Handler_RET(c);

	CHECK_EQ(c.PC, 0x2222);
	CHECK_EQ(c.SP, 1);

	Handler_RET(c);

	CHECK_EQ(c.PC, 0x1111);
	CHECK_EQ(c.SP, 0);

	CHECK_THROWS(Handler_RET(c));
}

TEST_CASE("Instruction: JP nnn")
{
	SContext c{};
	c.IR = 0x0123;

	Handler_JP_nnn(c);

	CHECK_EQ(c.PC, 0x123);
}

TEST_CASE("Instruction: CALL nnn")
{
	SContext c{};

	SUBCASE("First CALL")
	{
		c.IR = 0x0123;
		c.PC = 0x444;
		c.SP = 0;

		Handler_CALL_nnn(c);

		CHECK_EQ(c.PC, 0x123);
		CHECK_EQ(c.SP, 1);
		CHECK_EQ(c.Stack[0], 0x444);
	}

	SUBCASE("Second CALL")
	{
		c.IR = 0x0456;
		c.PC = 0x123;
		c.SP = 1;
		c.Stack[0] = 0x444;

		Handler_CALL_nnn(c);

		CHECK_EQ(c.PC, 0x456);
		CHECK_EQ(c.SP, 2);
		CHECK_EQ(c.Stack[0], 0x444);
		CHECK_EQ(c.Stack[1], 0x123);
	}

	SUBCASE("Stack full")
	{
		c.SP = gsl::narrow<std::uint8_t>(c.Stack.size());

		CHECK_THROWS(Handler_CALL_nnn(c));
	}
}

TEST_CASE("Instruction: SE Vx, kk")
{
	SContext c{};
	
	SUBCASE("Skip:    Vx == kk")
	{
		c.PC = 0;
		c.V[1] = 0x11;
		c.IR = 0x0111;

		Handler_SE_Vx_kk(c);

		CHECK_EQ(c.PC, InstructionByteSize);
	}

	SUBCASE("No Skip: Vx != kk")
	{
		c.PC = 0;
		c.V[1] = 0x11;
		c.IR = 0x0122;

		Handler_SE_Vx_kk(c);

		CHECK_EQ(c.PC, 0);
	}
}

TEST_CASE("Instruction: SNE Vx, kk")
{
	SContext c{};

	SUBCASE("No Skip: Vx == kk")
	{
		c.PC = 0;
		c.V[1] = 0x11;
		c.IR = 0x0111;

		Handler_SNE_Vx_kk(c);

		CHECK_EQ(c.PC, 0);
	}

	SUBCASE("Skip:    Vx != kk")
	{
		c.PC = 0;
		c.V[1] = 0x11;
		c.IR = 0x0122;

		Handler_SNE_Vx_kk(c);

		CHECK_EQ(c.PC, InstructionByteSize);
	}
}

TEST_CASE("Instruction: SE Vx, Vy")
{
	SContext c{};

	SUBCASE("Skip:    Vx == Vy")
	{
		c.PC = 0;
		c.V[1] = 0x11;
		c.V[2] = 0x11;
		c.IR = 0x0120;

		Handler_SE_Vx_Vy(c);

		CHECK_EQ(c.PC, InstructionByteSize);
	}

	SUBCASE("No Skip: Vx != Vy")
	{
		c.PC = 0;
		c.V[1] = 0x11;
		c.V[2] = 0x22;
		c.IR = 0x0120;

		Handler_SE_Vx_Vy(c);

		CHECK_EQ(c.PC, 0);
	}
}

TEST_CASE("Instruction: LD Vx, kk")
{
	SContext c{};

	c.V[1] = 0;
	c.IR = 0x0111;

	Handler_LD_Vx_kk(c);

	CHECK_EQ(c.V[1], 0x11);
}

TEST_CASE("Instruction: ADD Vx, kk")
{
	SContext c{};

	c.V[1] = 0x20;
	c.IR = 0x0110;

	Handler_ADD_Vx_kk(c);

	CHECK_EQ(c.V[1], 0x30);
}

TEST_CASE("Instruction: LD Vx, Vy")
{
	SContext c{};

	c.V[1] = 0x10;
	c.V[2] = 0x20;
	c.IR = 0x0120;

	Handler_LD_Vx_Vy(c);

	CHECK_EQ(c.V[1], 0x20);
	CHECK_EQ(c.V[2], 0x20);
}

TEST_CASE("Instruction: OR Vx, Vy")
{
	SContext c{};

	c.V[1] = 0x10;
	c.V[2] = 0x02;
	c.IR = 0x0120;

	Handler_OR_Vx_Vy(c);

	CHECK_EQ(c.V[1], 0x12);
	CHECK_EQ(c.V[2], 0x02);
}

TEST_CASE("Instruction: AND Vx, Vy")
{
	SContext c{};

	c.V[1] = 0x30;
	c.V[2] = 0x12;
	c.IR = 0x0120;

	Handler_AND_Vx_Vy(c);

	CHECK_EQ(c.V[1], 0x10);
	CHECK_EQ(c.V[2], 0x12);
}

TEST_CASE("Instruction: XOR Vx, Vy")
{
	SContext c{};

	c.V[1] = 0x30;
	c.V[2] = 0x12;
	c.IR = 0x0120;

	Handler_XOR_Vx_Vy(c);

	CHECK_EQ(c.V[1], 0x22);
	CHECK_EQ(c.V[2], 0x12);
}

TEST_CASE("Instruction: ADD Vx, Vy")
{
	SContext c{};

	SUBCASE("Without carry")
	{
		c.V[1] = 0x10;
		c.V[2] = 0x20;
		c.IR = 0x0120;

		Handler_ADD_Vx_Vy(c);

		CHECK_EQ(c.V[1], 0x30);
		CHECK_EQ(c.V[2], 0x20);
		CHECK_EQ(c.V[0xF], 0);
	}

	SUBCASE("With carry")
	{
		c.V[1] = 0xA0;
		c.V[2] = 0xB0;
		c.IR = 0x0120;

		Handler_ADD_Vx_Vy(c);

		CHECK_EQ(c.V[1], 0x50); // 0xA0 + 0xB0 = 0x150 
		CHECK_EQ(c.V[2], 0xB0);
		CHECK_EQ(c.V[0xF], 1);
	}

	SUBCASE("With carry (border)")
	{
		c.V[1] = 0x10;
		c.V[2] = 0xF0;
		c.IR = 0x0120;

		Handler_ADD_Vx_Vy(c);

		CHECK_EQ(c.V[1], 0); // 0xF0 + 0x10 = 0x100 
		CHECK_EQ(c.V[2], 0xF0);
		CHECK_EQ(c.V[0xF], 1);
	}
}

TEST_CASE("Instruction: SUB Vx, Vy")
{
	SContext c{};

	SUBCASE("Without borrow")
	{
		c.V[1] = 0x20;
		c.V[2] = 0x10;
		c.IR = 0x0120;

		Handler_SUB_Vx_Vy(c);

		CHECK_EQ(c.V[1], 0x10);
		CHECK_EQ(c.V[2], 0x10);
		CHECK_EQ(c.V[0xF], 1);
	}

	SUBCASE("With borrow")
	{
		c.V[1] = 0x10;
		c.V[2] = 0x20;
		c.IR = 0x0120;

		Handler_SUB_Vx_Vy(c);

		CHECK_EQ(c.V[1], 0xF0); // 0x10 - 0x20 = 0xF0 (wrapped around)
		CHECK_EQ(c.V[2], 0x20);
		CHECK_EQ(c.V[0xF], 0);
	}

	SUBCASE("Vx == Vy")
	{
		c.V[1] = 0x10;
		c.V[2] = 0x10;
		c.IR = 0x0120;

		Handler_SUB_Vx_Vy(c);

		CHECK_EQ(c.V[1], 0);
		CHECK_EQ(c.V[2], 0x10);
		CHECK_EQ(c.V[0xF], 0);
	}
}

TEST_CASE("Instruction: SHR Vx")
{
	SContext c{};

	SUBCASE("LSB is 0")
	{
		c.V[1] = 0x10;
		c.IR = 0x0100;

		Handler_SHR_Vx(c);

		CHECK_EQ(c.V[1], 0x08);
		CHECK_EQ(c.V[0xF], 0);
	}

	SUBCASE("LSB is 1")
	{
		c.V[1] = 0x11;
		c.IR = 0x0100;

		Handler_SHR_Vx(c);

		CHECK_EQ(c.V[1], 0x08);
		CHECK_EQ(c.V[0xF], 1);
	}
}

TEST_CASE("Instruction: SUBN Vx, Vy")
{
	SContext c{};

	SUBCASE("Without borrow")
	{
		c.V[1] = 0x10;
		c.V[2] = 0x20;
		c.IR = 0x0120;

		Handler_SUBN_Vx_Vy(c);

		CHECK_EQ(c.V[1], 0x10);
		CHECK_EQ(c.V[2], 0x20);
		CHECK_EQ(c.V[0xF], 1);
	}

	SUBCASE("With borrow")
	{
		c.V[1] = 0x20;
		c.V[2] = 0x10;
		c.IR = 0x0120;

		Handler_SUBN_Vx_Vy(c);

		CHECK_EQ(c.V[1], 0xF0); // 0x10 - 0x20 = 0xF0 (wrapped around)
		CHECK_EQ(c.V[2], 0x10);
		CHECK_EQ(c.V[0xF], 0);
	}

	SUBCASE("Vx == Vy")
	{
		c.V[1] = 0x10;
		c.V[2] = 0x10;
		c.IR = 0x0120;

		Handler_SUBN_Vx_Vy(c);

		CHECK_EQ(c.V[1], 0);
		CHECK_EQ(c.V[2], 0x10);
		CHECK_EQ(c.V[0xF], 0);
	}
}

TEST_CASE("Instruction: SHL Vx")
{
	SContext c{};

	SUBCASE("MSB is 0")
	{
		c.V[1] = 0x10;
		c.IR = 0x0100;

		Handler_SHL_Vx(c);

		CHECK_EQ(c.V[1], 0x20);
		CHECK_EQ(c.V[0xF], 0);
	}

	SUBCASE("MSB is 1")
	{
		c.V[1] = 0x90;
		c.IR = 0x0100;

		Handler_SHL_Vx(c);

		CHECK_EQ(c.V[1], 0x20);
		CHECK_EQ(c.V[0xF], 1);
	}
}

TEST_CASE("Instruction: SNE Vx, Vy")
{
	SContext c{};

	SUBCASE("No Skip: Vx == Vy")
	{
		c.PC = 0;
		c.V[1] = 0x10;
		c.V[2] = 0x10;
		c.IR = 0x0120;

		Handler_SNE_Vx_Vy(c);

		CHECK_EQ(c.PC, 0);
	}

	SUBCASE("Skip:    Vx != Vy")
	{
		c.PC = 0;
		c.V[1] = 0x10;
		c.V[2] = 0x20;
		c.IR = 0x0120;

		Handler_SNE_Vx_Vy(c);

		CHECK_EQ(c.PC, InstructionByteSize);
	}
}

TEST_CASE("Instruction: LD I, nnn")
{
	SContext c{};

	c.I = 0;
	c.IR = 0x0123;

	Handler_LD_I_nnn(c);

	CHECK_EQ(c.I, 0x123);
}

TEST_CASE("Instruction: JP V0, nnn")
{
	SContext c{};

	c.PC = 0;
	c.V[0] = 0x11;
	c.IR = 0x0222;

	Handler_JP_V0_nnn(c);

	CHECK_EQ(c.PC, 0x233);
}

TEST_CASE("Instruction: RND Vx, kk")
{
	constexpr std::size_t N{ 1000 }; // Number of times to run this test

	constexpr std::uint8_t kk{ 0b10100101 };
	constexpr std::uint8_t kkInv{ gsl::narrow_cast<std::uint8_t>(~kk) };

	SContext c{};
	c.IR = 0x0100 | kk;

	for (std::size_t i = 0; i < N; i++)
	{
		c.V[1] = 0;

		Handler_RND_Vx_kk(c);

		// check that all bits not set in `kk` are 0 in `Vx`
		CHECK_EQ(c.V[1] & kkInv, 0);
	}
}

// TODO: DRW Vx, Vy, n

TEST_CASE("Instruction: SKP Vx")
{
	SContext c{};

	SUBCASE("Skip:    Key pressed")
	{
		c.Keyboard[8] = true;
		c.V[1] = 8;
		c.IR = 0x0100;
		c.PC = 0;

		Handler_SKP_Vx(c);

		CHECK_EQ(c.PC, InstructionByteSize);
	}

	SUBCASE("No Skip: Key not pressed")
	{
		c.Keyboard[8] = false;
		c.V[1] = 8;
		c.IR = 0x0100;
		c.PC = 0;

		Handler_SKP_Vx(c);

		CHECK_EQ(c.PC, 0);
	}
}

TEST_CASE("Instruction: SKNP Vx")
{
	SContext c{};

	SUBCASE("Skip:    Key not pressed")
	{
		c.Keyboard[8] = false;
		c.V[1] = 8;
		c.IR = 0x0100;
		c.PC = 0;

		Handler_SKNP_Vx(c);

		CHECK_EQ(c.PC, InstructionByteSize);
	}

	SUBCASE("No Skip: Key pressed")
	{
		c.Keyboard[8] = true;
		c.V[1] = 8;
		c.IR = 0x0100;
		c.PC = 0;

		Handler_SKNP_Vx(c);

		CHECK_EQ(c.PC, 0);
	}
}

TEST_CASE("Instruction: LD Vx, DT")
{
	SContext c{};

	c.V[1] = 0;
	c.DT = 0x12;
	c.IR = 0x0100;

	Handler_LD_Vx_DT(c);

	CHECK_EQ(c.V[1], 0x12);
	CHECK_EQ(c.DT, 0x12);
}
