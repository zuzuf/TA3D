#ifndef __YUNI_VM_ASSEMBLY_H__
# define __YUNI_VM_ASSEMBLY_H__

# include "../yuni.h"
# include "std.h"
# include "fwd.h"
# include "program.h"


namespace Yuni
{
namespace VM
{

	class Assembly
	{
	public:
	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Default constructor
		*/
		Assembly();
		//! Destructor
		~Assembly();
		//@}


		//! \name Execution flow
		//@{
		/*!
		** \brief Check the validity of the assembly code
		*/
		bool validate() const;

		/*!
		** \brief Execute the program
		*/
		int execute();
		//@}


		//! \name Intrinsic functions
		//@{
		//! intrinsic function
		void intrinsic();
		//! intrinsic function, with 1 parameter
		void intrinsic(uint8 r1);
		//! intrinsic function, with 2 parameters
		void intrinsic(uint8 r1, uint8 r2);
		//! intrinsic function, with 3 parameters
		void intrinsic(uint8 r1, uint8 r2, uint8 r3);
		//! intrinsic function, with 4 parameters
		void intrinsic(uint8 r1, uint8 r2, uint8 r3, uint8 r4);
		//@}


		//! \name Add
		//@{
		//! add <ret>, <R1>, <R2>: <ret> = <R1> + <R2>
		void add(uint8 ret, uint8 r1, uint8 r2);
		//! addu <ret>, <R1>, <R2>: <ret> = <R1> + <R2>  (unsigned)
		void addu(uint8 ret, uint8 r1, uint8 r2);
		//! addi <ret>, <R1>, I: <ret> = <R1> + I
		void addi(uint8 ret, uint8 r1, sint64 i);
		//! addui <ret>, <R1>, I: <ret> = <R1> + I (unsigned)
		void addui(uint8 ret, uint8 r1, uint64 i);
		//@}


		//! \name Misc. functions
		//@{
		//! nop
		void nop();

		//! Abort the execution of the program
		void exit();
		//! Set the value of the exit code from the value of a register
		void exitCode(uint8 r1);
		//! Set the value of the exit code from an immediate value
		void exitCodei(sint8 i);
		//@}


	private:
		Yuni::Private::VM::Program pProgram;

	}; // class Assembly




} // namespace VM
} // namespace Yuni

#endif // __YUNI_VM_ASSEMBLY_H__
