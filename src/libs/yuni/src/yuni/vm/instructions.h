#ifndef __YUNI_VM_INSTRUCTIONS_H__
# define __YUNI_VM_INSTRUCTIONS_H__

namespace Yuni
{
namespace Private
{
namespace VM
{
namespace Instruction
{

	enum Index
	{
		exit = 0,
		intrinsic,
		add,
		addu,
		addi,
		addui,
		nop,
		exitCode,
		exitCodei,
		max
	};



} // namespace Instruction
} // namespace VM
} // namespace Private
} // namespace Yuni

#endif // __YUNI_VM_INSTRUCTIONS_H__
