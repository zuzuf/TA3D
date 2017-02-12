#ifndef __YUNI_VM_PROGRAM_HXX__
# define __YUNI_VM_PROGRAM_HXX__



namespace Yuni
{
namespace Private
{
namespace VM
{

	namespace // anonymous
	{
		template<class T>
		struct OperandConverter
		{
			enum
			{
				byteCount = sizeof(T),
			};

			static void Write(char* data, unsigned int& count, T value)
			{
				*(reinterpret_cast<T*>(data + count)) = value;
				count += byteCount;
			}
		};

	} // anonymous namespace



	inline void Program::add(InstructionType instruction)
	{
		if (instructionCount + 1 > instructionCapacity)
			increaseInstructionCapacity();
		instructions[instructionCount] = instruction;
		++instructionCount;
	}


	template<class T1>
	inline void Program::add(InstructionType instruction, T1 r1)
	{
		if (instructionCount + 1 > instructionCapacity)
			increaseInstructionCapacity();
		instructions[instructionCount] = instruction;
		++instructionCount;

		enum
		{
			byteCount = OperandConverter<T1>::byteCount,
		};
		if (operandCount + byteCount > operandCapacity)
			increaseOperandCapacity();
		OperandConverter<T1>::Write(operands, operandCount, r1);
	}


	template<class T1, class T2>
	inline void Program::add(InstructionType instruction, T1 r1, T2 r2)
	{
		if (instructionCount + 1 > instructionCapacity)
			increaseInstructionCapacity();
		instructions[instructionCount] = instruction;
		++instructionCount;

		enum
		{
			byteCount = OperandConverter<T1>::byteCount + OperandConverter<T2>::byteCount,
		};
		if (operandCount + byteCount > operandCapacity)
			increaseOperandCapacity();
		OperandConverter<T1>::Write(operands, operandCount, r1);
		OperandConverter<T2>::Write(operands, operandCount, r2);
	}


	template<class T1, class T2, class T3>
	inline void Program::add(InstructionType instruction, T1 r1, T2 r2, T3 r3)
	{
		if (instructionCount + 1 > instructionCapacity)
			increaseInstructionCapacity();
		instructions[instructionCount] = instruction;
		++instructionCount;

		enum
		{
			byteCount = OperandConverter<T1>::byteCount + OperandConverter<T2>::byteCount
				+ OperandConverter<T3>::byteCount,
		};
		if (operandCount + byteCount > operandCapacity)
			increaseOperandCapacity();
		OperandConverter<T1>::Write(operands, operandCount, r1);
		OperandConverter<T2>::Write(operands, operandCount, r2);
		OperandConverter<T3>::Write(operands, operandCount, r3);
	}


	template<class T1, class T2, class T3, class T4>
	inline void Program::add(InstructionType instruction, T1 r1, T2 r2, T3 r3, T4 r4)
	{
		if (instructionCount + 1 > instructionCapacity)
			increaseInstructionCapacity();
		instructions[instructionCount] = instruction;
		++instructionCount;

		enum
		{
			byteCount = OperandConverter<T1>::byteCount + OperandConverter<T2>::byteCount
				+ OperandConverter<T3>::byteCount + OperandConverter<T4>::byteCount,
		};
		if (operandCount + byteCount > operandCapacity)
			increaseOperandCapacity();
		OperandConverter<T1>::Write(operands, operandCount, r1);
		OperandConverter<T2>::Write(operands, operandCount, r2);
		OperandConverter<T3>::Write(operands, operandCount, r3);
		OperandConverter<T4>::Write(operands, operandCount, r4);
	}


	template<class T1, class T2, class T3, class T4, class T5>
	inline void Program::add(InstructionType instruction, T1 r1, T2 r2, T3 r3, T4 r4, T5 r5)
	{
		if (instructionCount + 1 > instructionCapacity)
			increaseInstructionCapacity();
		instructions[instructionCount] = instruction;
		++instructionCount;

		enum
		{
			byteCount = OperandConverter<T1>::byteCount + OperandConverter<T2>::byteCount
				+ OperandConverter<T3>::byteCount + OperandConverter<T4>::byteCount
				+ OperandConverter<T5>::byteCount,
		};
		if (operandCount + byteCount > operandCapacity)
			increaseOperandCapacity();
		OperandConverter<T1>::Write(operands, operandCount, r1);
		OperandConverter<T2>::Write(operands, operandCount, r2);
		OperandConverter<T3>::Write(operands, operandCount, r3);
		OperandConverter<T4>::Write(operands, operandCount, r4);
		OperandConverter<T5>::Write(operands, operandCount, r5);
	}




} // namespace VM
} // namespace Private
} // namespace Yuni

#endif // __YUNI_VM_PROGRAM_HXX__
