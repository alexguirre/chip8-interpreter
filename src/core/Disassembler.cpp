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
		// if the address was already disassembled, return
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

			switch (inst.Kind)
			{
			case EInstructionKind::Other: break; // nothing to do
			case EInstructionKind::Jump:
			{
				if (inst.Opcode == 0x1000) // if it is `JP nnn`
				{
					// explore the destination address
					Disassemble(opcode.NNN(), mem, dest);
				}
				else if (inst.Opcode == 0xB000) // if it is `JP V0, nnn`
				{
					// with `JP V0, nnn`, it is not really possible to know the destination address
					// since it depends on the runtime value of `V0`, so for now we don't explore
					// the jump
				}

				// the jump is always perform, so we reached the end of this branch
				return;
			}
			case EInstructionKind::Branch:
			{
				if (inst.Name == "CALL")
				{
					// explore the call destination address
					Disassemble(opcode.NNN(), mem, dest);
				}
				else if (inst.Name == "SE" || inst.Name == "SNE" || inst.Name == "SKP" ||
						 inst.Name == "SKNP")
				{
					// skip one instruction and explore the following one
					Disassemble(addr + constants::InstructionByteSize * 2, mem, dest);
				}

				break;
			}
			case EInstructionKind::Return:
			case EInstructionKind::Exit:
			{
				// if we are at a RET or an EXIT, we reached the end of this branch
				return;
			}
			default: throw std ::runtime_error("Invalid instruction kind");
			}

			// move to next instruction
			Disassemble(addr + constants::InstructionByteSize, mem, dest);
		}
	}
}