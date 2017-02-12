#ifndef __YUNI_VM_PROGRAM_H__
# define __YUNI_VM_PROGRAM_H__

# include "../yuni.h"
# include "std.h"
# include "fwd.h"


namespace Yuni
{
namespace Private
{
namespace VM
{

	typedef Yuni::VM::InstructionType  InstructionType;



	class Program
	{
	public:

	public:
		Program();
		~Program();

		void clear();

		void add(InstructionType instruction);

		template<class T1>
		void add(InstructionType instruction, T1 r1);

		template<class T1, class T2>
		void add(InstructionType instruction, T1 r1, T2 r2);

		template<class T1, class T2, class T3>
		void add(InstructionType instruction, T1 r1, T2 r2, T3 r3);

		template<class T1, class T2, class T3, class T4>
		void add(InstructionType instruction, T1 r1, T2 r2, T3 r3, T4 r4);

		template<class T1, class T2, class T3, class T4, class T5>
		void add(InstructionType instruction, T1 r1, T2 r2, T3 r3, T4 r4, T5 r5);

		void reserveInstructions(unsigned int count);
		void reserveOperands(unsigned int count);

		/*!
		** \brief Validate the assembly code
		*/
		bool validate() const;

		/*!
		** \brief Execute the program
		**
		** The program should be validated first
		*/
		int execute();

	protected:
		void increaseInstructionCapacity();
		void increaseInstructionCapacity(unsigned int chunkSize);
		void increaseOperandCapacity();
		void increaseOperandCapacity(unsigned int chunkSize);

	public:
		//! Continuous list of instructions
		InstructionType* instructions;
		//! The number of instructions
		unsigned int instructionCount;
		//! The capacity
		unsigned int instructionCapacity;
		//! Continuous list of operands for each instructions
		char* operands;
		//! The number of operands
		unsigned int operandCount;
		//!
		unsigned int operandCapacity;

	}; // class Program





} // namespace VM
} // namespace Private
} // namespace Yuni

# include "program.hxx"

#endif // __YUNI_VM_PROGRAM_H__
