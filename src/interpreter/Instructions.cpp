#include "Instructions.h"
#include <sstream>
#include <random>
#include "Interpreter.h"

static void CLS_Handler(SContext& c)
{
	std::fill(c.PixelBuffer.begin(), c.PixelBuffer.end(), std::uint8_t(0));
	c.PixelBufferDirty = true;
}
static std::string CLS_ToString(SContext&) { return "CLS"; }

static void RET_Handler(SContext& c)
{
	c.PC = c.Stack[--c.SP];
}
static std::string RET_ToString(SContext&) { return "RET"; }

static void JP_Handler(SContext& c)
{
	c.PC = c.NNN();
}
static std::string JP_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "JP " << std::hex << std::setfill('0') << std::setw(3) << static_cast<std::uint32_t>(c.NNN());
	return ss.str();
}

static void CALL_Handler(SContext& c)
{
	c.Stack[c.SP++] = c.PC;
	c.PC = c.NNN();
}
static std::string CALL_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "CALL " << std::hex << std::setfill('0') << std::setw(3) << static_cast<std::uint32_t>(c.NNN());
	return ss.str();
}

static void SE_Handler(SContext& c)
{
	if (c.V[c.X()] == c.KK())
	{
		c.PC += 2;
	}
}
static std::string SE_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "SE V" << std::hex << static_cast<std::uint32_t>(c.X()) << ", " << std::hex << std::setfill('0') << std::setw(2) << static_cast<std::uint32_t>(c.KK());
	return ss.str();
}

static void SNE_Handler(SContext& c)
{
	if (c.V[c.X()] != c.KK())
	{
		c.PC += 2;
	}
}
static std::string SNE_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "SNE V" << std::hex << static_cast<std::uint32_t>(c.X()) << ", " << std::hex << std::setfill('0') << std::setw(2) << static_cast<std::uint32_t>(c.KK());
	return ss.str();
}

static void SE_2_Handler(SContext& c)
{
	if (c.V[c.X()] == c.V[c.Y()])
	{
		c.PC += 2;
	}
}
static std::string SE_2_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "SE V" << std::hex << static_cast<std::uint32_t>(c.X()) << ", V" << std::hex << static_cast<std::uint32_t>(c.Y());
	return ss.str();
}

static void LD_Handler(SContext& c)
{
	c.V[c.X()] = c.KK();
}
static std::string LD_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "LD V" << std::hex << static_cast<std::uint32_t>(c.X()) << ", " << std::hex << std::setfill('0') << std::setw(2) << static_cast<std::uint32_t>(c.KK());
	return ss.str();
}

static void ADD_Handler(SContext& c)
{
	c.V[c.X()] += c.KK();
}
static std::string ADD_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "ADD V" << std::hex << static_cast<std::uint32_t>(c.X()) << ", " << std::hex << std::setfill('0') << std::setw(2) << static_cast<std::uint32_t>(c.KK());
	return ss.str();
}

static void LD_2_Handler(SContext& c)
{
	c.V[c.X()] = c.V[c.Y()];
}
static std::string LD_2_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "LD V" << std::hex << static_cast<std::uint32_t>(c.X()) << ", V" << std::hex << static_cast<std::uint32_t>(c.Y());
	return ss.str();
}

static void OR_Handler(SContext& c)
{
	c.V[c.X()] |= c.V[c.Y()];
}
static std::string OR_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "OR V" << std::hex << static_cast<std::uint32_t>(c.X()) << ", V" << std::hex << static_cast<std::uint32_t>(c.Y());
	return ss.str();
}

static void AND_Handler(SContext& c)
{
	c.V[c.X()] &= c.V[c.Y()];
}
static std::string AND_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "AND V" << std::hex << static_cast<std::uint32_t>(c.X()) << ", V" << std::hex << static_cast<std::uint32_t>(c.Y());
	return ss.str();
}

static void XOR_Handler(SContext& c)
{
	c.V[c.X()] ^= c.V[c.Y()];
}
static std::string XOR_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "XOR V" << std::hex << static_cast<std::uint32_t>(c.X()) << ", V" << std::hex << static_cast<std::uint32_t>(c.Y());
	return ss.str();
}

static void ADD_2_Handler(SContext& c)
{
	std::uint8_t& vx = c.V[c.X()];
	std::uint8_t& vy = c.V[c.Y()];
	c.V[0xF] = static_cast<std::int32_t>(vx) + static_cast<std::int32_t>(vy) > std::numeric_limits<std::uint8_t>::max();
	vx += vy;
}
static std::string ADD_2_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "ADD V" << std::hex << static_cast<std::uint32_t>(c.X()) << ", V" << std::hex << static_cast<std::uint32_t>(c.Y());
	return ss.str();
}

static void SUB_Handler(SContext& c)
{
	std::uint8_t& vx = c.V[c.X()];
	std::uint8_t& vy = c.V[c.Y()];
	c.V[0xF] = vx > vy;
	vx -= vy;
}
static std::string SUB_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "SUB V" << std::hex << static_cast<std::uint32_t>(c.X()) << ", V" << std::hex << static_cast<std::uint32_t>(c.Y());
	return ss.str();
}

static void SHR_Handler(SContext& c)
{
	std::uint8_t& vx = c.V[c.X()];
	c.V[0xF] = vx & 1;
	vx >>= 1;
}
static std::string SHR_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "SHR V" << std::hex << static_cast<std::uint32_t>(c.X());
	return ss.str();
}

static void SUBN_Handler(SContext& c)
{
	std::uint8_t& vx = c.V[c.X()];
	std::uint8_t& vy = c.V[c.Y()];
	c.V[0xF] = vy > vx;
	vx = vy - vx;
}
static std::string SUBN_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "SUBN V" << std::hex << static_cast<std::uint32_t>(c.X()) << ", V" << std::hex << static_cast<std::uint32_t>(c.Y());
	return ss.str();
}

static void SHL_Handler(SContext& c)
{
	std::uint8_t& vx = c.V[c.X()];
	c.V[0xF] = (vx >> 7) & 1;
	vx <<= 1;
}
static std::string SHL_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "SHL V" << std::hex << static_cast<std::uint32_t>(c.X());
	return ss.str();
}

static void SNE_2_Handler(SContext& c)
{
	if (c.V[c.X()] != c.V[c.Y()])
	{
		c.PC += 2;
	}
}
static std::string SNE_2_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "SNE V" << std::hex << static_cast<std::uint32_t>(c.X()) << ", V" << std::hex << static_cast<std::uint32_t>(c.Y());
	return ss.str();
}

static void LD_I_Handler(SContext& c)
{
	c.I = c.NNN();
}
static std::string LD_I_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "LD I, " << std::hex << std::setfill('0') << std::setw(3) << static_cast<std::uint32_t>(c.NNN());
	return ss.str();
}

static void JP_2_Handler(SContext& c)
{
	c.PC = c.NNN() + c.V[0];
}
static std::string JP_2_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "JP V0, " << std::hex << std::setfill('0') << std::setw(3) << static_cast<std::uint32_t>(c.NNN());
	return ss.str();
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
static std::string RND_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "RND V" << std::hex << static_cast<std::uint32_t>(c.X()) << ", " << std::hex << std::setfill('0') << std::setw(2) << static_cast<std::uint32_t>(c.KK());
	return ss.str();
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
static std::string DRW_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "DRW V" << std::hex << static_cast<std::uint32_t>(c.X()) << ", V" << std::hex << static_cast<std::uint32_t>(c.Y()) << ", " << std::hex << static_cast<std::uint32_t>(c.N());
	return ss.str();
}

static void SKP_Handler(SContext& c)
{
	std::uint8_t vx = c.V[c.X()];
	if (c.Keyboard[vx])
	{
		c.PC += 2;
	}
}
static std::string SKP_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "SKP V" << std::hex << static_cast<std::uint32_t>(c.X());
	return ss.str();
}

static void SKNP_Handler(SContext& c)
{
	std::uint8_t vx = c.V[c.X()];
	if (!c.Keyboard[vx])
	{
		c.PC += 2;
	}
}
static std::string SKNP_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "SKNP V" << std::hex << static_cast<std::uint32_t>(c.X());
	return ss.str();
}

static void LD_3_Handler(SContext& c)
{
	c.V[c.X()] = c.DT;
}
static std::string LD_3_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "LD V" << std::hex << static_cast<std::uint32_t>(c.X()) << ", DT";
	return ss.str();
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
	c.PC -= 2;
}
static std::string LD_4_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "LD V" << std::hex << static_cast<std::uint32_t>(c.X()) << ", K";
	return ss.str();
}

static void LD_5_Handler(SContext& c)
{
	c.DT = c.V[c.X()];
}
static std::string LD_5_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "LD DT, V" << std::hex << static_cast<std::uint32_t>(c.X());
	return ss.str();
}

static void LD_6_Handler(SContext& c)
{
	c.ST = c.V[c.X()];
}
static std::string LD_6_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "LD ST, V" << std::hex << static_cast<std::uint32_t>(c.X());
	return ss.str();
}

static void ADD_I_Handler(SContext& c)
{
	std::uint8_t& vx = c.V[c.X()];
	c.V[0xF] = static_cast<std::int32_t>(vx) + static_cast<std::int32_t>(c.I) > std::numeric_limits<std::uint8_t>::max();
	c.I += vx;
}
static std::string ADD_I_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "ADD I, V" << std::hex << static_cast<std::uint32_t>(c.X());
	return ss.str();
}

static void LD_7_Handler(SContext& c)
{
	c.I = CInterpreter::FontsetCharByteSize * c.V[c.X()];
}
static std::string LD_7_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "LD F, V" << std::hex << static_cast<std::uint32_t>(c.X());
	return ss.str();
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
static std::string LD_8_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "LD B, V" << std::hex << static_cast<std::uint32_t>(c.X());
	return ss.str();
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
static std::string LD_9_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "LD [I], V" << std::hex << static_cast<std::uint32_t>(c.X());
	return ss.str();
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
static std::string LD_10_ToString(SContext& c)
{
	std::ostringstream ss;
	ss << "LD V" << std::hex << static_cast<std::uint32_t>(c.X()) << ", [I]";
	return ss.str();
}

const std::vector<SInstruction> SInstruction::InstructionSet =
{
	{ "CLS",	CLS_Handler,	0x00E0,	0xF0FF,	CLS_ToString	},
	{ "RET",	RET_Handler,	0x00EE,	0xF0FF,	RET_ToString	},
	{ "JP",		JP_Handler,		0x1000,	0xF000,	JP_ToString		},
	{ "CALL",	CALL_Handler,	0x2000,	0xF000,	CALL_ToString	},
	{ "SE",		SE_Handler,		0x3000,	0xF000,	SE_ToString		},
	{ "SNE",	SNE_Handler,	0x4000,	0xF000,	SNE_ToString	},
	{ "SE",		SE_2_Handler,	0x5000,	0xF00F,	SE_2_ToString	},
	{ "LD",		LD_Handler,		0x6000,	0xF000,	LD_ToString		},
	{ "ADD",	ADD_Handler,	0x7000,	0xF000,	ADD_ToString	},
	{ "LD",		LD_2_Handler,	0x8000,	0xF00F,	LD_2_ToString	},
	{ "OR",		OR_Handler,		0x8001,	0xF00F,	OR_ToString		},
	{ "AND",	AND_Handler,	0x8002,	0xF00F,	AND_ToString	},
	{ "XOR",	XOR_Handler,	0x8003,	0xF00F,	XOR_ToString	},
	{ "ADD",	ADD_2_Handler,	0x8004,	0xF00F,	ADD_2_ToString	},
	{ "SUB",	SUB_Handler,	0x8005,	0xF00F,	SUB_ToString	},
	{ "SHR",	SHR_Handler,	0x8006,	0xF00F,	SHR_ToString	},
	{ "SUBN",	SUBN_Handler,	0x8007,	0xF00F,	SUBN_ToString	},
	{ "SHL",	SHL_Handler,	0x800E,	0xF00F,	SHL_ToString	},
	{ "SNE",	SNE_2_Handler,	0x9000,	0xF00F,	SNE_2_ToString	},
	{ "LD",		LD_I_Handler,	0xA000,	0xF000,	LD_I_ToString	},
	{ "JP",		JP_2_Handler,	0xB000,	0xF000,	JP_2_ToString	},
	{ "RND",	RND_Handler,	0xC000,	0xF000,	RND_ToString	},
	{ "DRW",	DRW_Handler,	0xD000,	0xF000,	DRW_ToString	},
	{ "SKP",	SKP_Handler,	0xE09E,	0xF0FF,	SKP_ToString	},
	{ "SKNP",	SKNP_Handler,	0xE0A1,	0xF0FF,	SKNP_ToString	},
	{ "LD",		LD_3_Handler,	0xF007,	0xF0FF,	LD_3_ToString	},
	{ "LD",		LD_4_Handler,	0xF00A,	0xF0FF,	LD_4_ToString	},
	{ "LD",		LD_5_Handler,	0xF015,	0xF0FF,	LD_5_ToString	},
	{ "LD",		LD_6_Handler,	0xF018,	0xF0FF,	LD_6_ToString	},
	{ "ADD",	ADD_I_Handler,	0xF01E,	0xF0FF,	ADD_I_ToString	},
	{ "LD",		LD_7_Handler,	0xF029,	0xF0FF,	LD_7_ToString	},
	{ "LD",		LD_8_Handler,	0xF033,	0xF0FF,	LD_8_ToString	},
	{ "LD",		LD_9_Handler,	0xF055,	0xF0FF,	LD_9_ToString	},
	{ "LD",		LD_10_Handler,	0xF065,	0xF0FF,	LD_10_ToString	},
};
