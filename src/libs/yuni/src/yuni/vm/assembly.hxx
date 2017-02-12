#ifndef __YUNI_VM_ASSEMBLY_HXX__
# define __YUNI_VM_ASSEMBLY_HXX__



namespace Yuni
{
namespace VM
{

	inline void Assembly::nop()
	{
		pProgram.add(Private::VM::Instruction::nop);
	}

	inline void Assembly::exit()
	{
		pProgram.add(Private::VM::Instruction::exit);
	}

	inline void Assembly::exitCode(uint8 r1)
	{
		pProgram.add(Private::VM::Instruction::exitCode, r1);
	}

	inline void Assembly::exitCodei(sint8 i)
	{
		pProgram.add(Private::VM::Instruction::exitCode, i);
	}

	inline void Assembly::intrinsic()
	{
		pProgram.add(Private::VM::Instruction::intrinsic, static_cast<uint8>(0));
	}

	inline void Assembly::intrinsic(uint8 r1)
	{
		pProgram.add(Private::VM::Instruction::intrinsic, static_cast<uint8>(1), r1);
	}

	inline void Assembly::intrinsic(uint8 r1, uint8 r2)
	{
		pProgram.add(Private::VM::Instruction::intrinsic, static_cast<uint8>(2), r1, r2);
	}

	inline void Assembly::intrinsic(uint8 r1, uint8 r2, uint8 r3)
	{
		pProgram.add(Private::VM::Instruction::intrinsic, static_cast<uint8>(3), r1, r2, r3);
	}

	inline void Assembly::intrinsic(uint8 r1, uint8 r2, uint8 r3, uint8 r4)
	{
		pProgram.add(Private::VM::Instruction::intrinsic, static_cast<uint8>(4), r1, r2, r3, r4);
	}




	inline void Assembly::add(uint8 ret, uint8 r1, uint8 r2)
	{
		pProgram.add(Private::VM::Instruction::add, ret, r1, r2);
	}

	inline void Assembly::addu(uint8 ret, uint8 r1, uint8 r2)
	{
		pProgram.add(Private::VM::Instruction::addu, ret, r1, r2);
	}

	inline void Assembly::addi(uint8 ret, uint8 r1, sint64 i)
	{
		pProgram.add(Private::VM::Instruction::addi, ret, r1, i);
	}

	inline void Assembly::addui(uint8 ret, uint8 r1, uint64 i)
	{
		pProgram.add(Private::VM::Instruction::addui, ret, r1, i);
	}



	inline bool Assembly::validate() const
	{
		return pProgram.validate();
	}


	inline int execute()
	{
		return pProgram.execute();
	}



} // namespace VM
} // namespace Yuni

#endif // __YUNI_VM_ASSEMBLY_HXX__
