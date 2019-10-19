#include "Disassembler.h"
#include "Constants.h"
#include "Instructions.h"
#include "TypeAliases.h"
#include <gsl/gsl_util>

namespace c8
{
	SDisassembly CDisassembler::Disassemble(std::size_t startAddress, const SMemory& memory) const
	{
		SDisassembly d;
		Disassemble(startAddress, memory, d);
		std::sort(d.begin(), d.end(), [](auto a, auto b) { return a.Address < b.Address; });
		return d;
	}

	void CDisassembler::Disassemble(std::size_t addr, const SMemory& mem, SDisassembly& dest) const
	{
		if (std::any_of(dest.begin(), dest.end(), [addr](auto l) { return l.Address == addr; }))
		{
			return;
		}

		const SOpCode opcode = mem[addr] << 8 | mem[addr + 1];
		const optional_cref<SInstruction> instOpt = SInstruction::TryFindInstruction(opcode);
		if (instOpt.has_value())
		{
			const SInstruction& inst = instOpt.value().get();
			dest.emplace_back(addr, opcode, inst.ToString(inst, opcode));

			if (inst.Kind == EInstructionKind::Branch)
			{
				// TODO: explore the branch alternative path
			}
			else if (inst.Kind == EInstructionKind::Return || inst.Kind == EInstructionKind::Exit)
			{
				// if we are at a RET or an EXIT, we reached the end of this branch
				return;
			}

			// move to next instruction
			Disassemble(addr + constants::InstructionByteSize, mem, dest);
		}
	}
}