
#include "program.h"
#include "instructions.h"
#include <cstdlib>
#include <cstring>


namespace Yuni
{
namespace Private
{
namespace VM
{

	namespace // anonymous
	{

		class ProcessorData
		{
		public:
			ProcessorData()
				:exitCode(0)
			{
				memset(gpr, 0, sizeof(gpr));
				memset(spr, 0, sizeof(spr));
				memset(dpr, 0, sizeof(dpr));
			}

		public:
			//! General purpose registers
			uint64 gpr[16];
			//! Single-precision registers
			float spr[16];
			//! Double-precision registers
			double dpr[16];
			//! Exit code
			int exitCode;

		}; // class ProcessorData


	} // anonymous namespace



	int Program::execute()
	{
		// Implementation : Direct dispatch
		// Next instruction
		# define NEXT  goto **vp++

		// Converting each instructions into a list of goto jump
		void** jumps;
		{
			const unsigned int count = instructionCount;
			if (!count)
				return 0;
			jumps = (void**)::malloc(sizeof(void**) * (count + 1));
			void* const aliases[Instruction::max] =
			{
				&&j_exit,  // exit
				&&j_intrinsic,    // intrinsic
				&&j_add,
				&&j_addu,
				&&j_addi,
				&&j_addui,
				&&j_nop,
				&&j_exitcode,
				&&j_exitcodei
			};
			// We assume here that all instructions are valid
			// The first instruction will always be the 'exit' instruction
			jumps[0] = &&j_exit;
			for (unsigned int i = 0; i != count; ++i)
				jumps[i + 1] = aliases[instructions[i]];
		}

		// data for our virtual processor
		ProcessorData data;
		// pointer, on the first instruction
		void** vp = ((void**) jumps) + 1;
		// The current operand
		unsigned int op = 0;

		// execute the first instruction
		NEXT;

		// implementations of all instructions
		j_intrinsic:
			{
				const unsigned int params = operands[op++];
				// FIXME
				op += params;
				NEXT;
			}

		j_add:
			{
				const unsigned int ret = operands[op++];
				const unsigned int r1  = operands[op++];
				const unsigned int r2  = operands[op++];
				*((sint64*)(data.gpr) + ret) = (sint64)(data.gpr[r1]) + (sint64)(data.gpr[r2]);
				NEXT;
			}
		j_addu:
			{
				const unsigned int ret = operands[op++];
				const unsigned int r1  = operands[op++];
				const unsigned int r2  = operands[op++];
				data.gpr[ret] = data.gpr[r1] + data.gpr[r2];
				NEXT;
			}
		j_addi:
			{
				const unsigned int ret = operands[op++];
				const unsigned int r1  = operands[op++];
				const sint64 i         = *((sint64*)(operands + op));
				op += 8;
				*((sint64*)(data.gpr) + ret) = (sint64)(data.gpr[r1]) + i;
				NEXT;
			}
		j_addui:
			{
				const unsigned int ret = operands[op++];
				const unsigned int r1  = operands[op++];
				const uint64 i         = *((uint64*)(operands + op));
				op += 8;
				data.gpr[ret] = data.gpr[r1] + i;
				NEXT;
			}
		j_nop:
			{
				// do nothing on purpose
				NEXT;
			}
		j_exitcode:
			{
				const unsigned int r1 = operands[op++];
				data.exitCode = (int) data.gpr[r1];
				NEXT;
			}
		j_exitcodei:
			{
				data.exitCode = (int) operands[op++];
				NEXT;
			}

		j_exit:
		(void)::free(jumps);
		return data.exitCode;
		# undef NEXT
	}





} // namespace VM
} // namespace Private
} // namespace Yuni

