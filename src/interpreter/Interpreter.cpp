#include "Interpreter.h"
#include <algorithm>
#include <fstream>
#include <iterator>
#include <iostream>

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
}

void CInterpreter::LoadProgram(const std::filesystem::path& filePath)
{
	if (!fs::is_regular_file(filePath))
	{
		throw std::invalid_argument("Path '" + filePath.string() + "' is an invalid file");
	}

	mMemory.Clear();

	std::basic_ifstream<std::uint8_t> file(filePath, std::ios::in | std::ios::binary);
	mMemory.BulkWrite(
		ProgramStartAddress,
		std::istreambuf_iterator(file),
		std::istreambuf_iterator<std::uint8_t>()
	);

	mContext.Reset();
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

static void CLS_Handler(SContext&) { std::cout << "CLS_Handler\n"; }
static void RET_Handler(SContext&) { std::cout << "RET_Handler\n"; }
static void JP_Handler(SContext&) { std::cout << "JP_Handler\n"; }
static void CALL_Handler(SContext&) { std::cout << "CALL_Handler\n"; }
static void SE_Handler(SContext&) { std::cout << "SE_Handler\n"; }
static void SNE_Handler(SContext&) { std::cout << "SNE_Handler\n"; }
static void SE_2_Handler(SContext&) { std::cout << "SE_2_Handler\n"; }
static void LD_Handler(SContext&) { std::cout << "LD_Handler\n"; }
static void ADD_Handler(SContext&) { std::cout << "ADD_Handler\n"; }
static void LD_2_Handler(SContext&) { std::cout << "LD_2_Handler\n"; }
static void OR_Handler(SContext&) { std::cout << "OR_Handler\n"; }
static void AND_Handler(SContext&) { std::cout << "AND_Handler\n"; }
static void XOR_Handler(SContext&) { std::cout << "XOR_Handler\n"; }
static void ADD_2_Handler(SContext&) { std::cout << "ADD_2_Handler\n"; }
static void SUB_Handler(SContext&) { std::cout << "SUB_Handler\n"; }
static void SHR_Handler(SContext&) { std::cout << "SHR_Handler\n"; }
static void SUBN_Handler(SContext&) { std::cout << "SUBN_Handler\n"; }
static void SHL_Handler(SContext&) { std::cout << "SHL_Handler\n"; }
static void SNE_2_Handler(SContext&) { std::cout << "SNE_2_Handler\n"; }
static void LD_I_Handler(SContext&) { std::cout << "LD_I_Handler\n"; }
static void JP_2_Handler(SContext&) { std::cout << "JP_2_Handler\n"; }
static void RND_Handler(SContext&) { std::cout << "RND_Handler\n"; }
static void DRW_Handler(SContext&) { std::cout << "DRW_Handler\n"; }
static void SKP_Handler(SContext&) { std::cout << "SKP_Handler\n"; }
static void SKNP_Handler(SContext&) { std::cout << "SKNP_Handler\n"; }
static void LD_3_Handler(SContext&) { std::cout << "LD_3_Handler\n"; }
static void LD_4_Handler(SContext&) { std::cout << "LD_4_Handler\n"; }
static void LD_5_Handler(SContext&) { std::cout << "LD_5_Handler\n"; }
static void LD_6_Handler(SContext&) { std::cout << "LD_6_Handler\n"; }
static void ADD_I_Handler(SContext&) { std::cout << "ADD_I_Handler\n"; }
static void LD_7_Handler(SContext&) { std::cout << "LD_7_Handler\n"; }
static void LD_8_Handler(SContext&) { std::cout << "LD_8_Handler\n"; }
static void LD_9_Handler(SContext&) { std::cout << "LD_9_Handler\n"; }
static void LD_10_Handler(SContext&) { std::cout << "LD_10_Handler\n"; }

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
