#ifndef __YUNI_CORE_MATH_BASE_H__
# define __YUNI_CORE_MATH_BASE_H__


namespace Yuni
{
namespace Math
{
namespace Base
{


	template<int Nth = 10>
	struct N
	{
		//! Digits
		static const char* Digit() {return "0123456789abcdefghijklmnopqrstuvwxyz";}
		enum
		{
			//! the nth power of b
			n = Nth,
		};
	};


	typedef N<10>  Decimal;
	typedef N<8>   Octal;
	typedef N<16>  Hexa;



	struct HexaLowercase
	{
		//! Digits
		static const char* Digit() {return "0123456789abcdef";}
		enum
		{
			//! the nth power of b
			n = 16,
		};
	};


	struct HexaUppercase
	{
		//! Digits
		static const char* Digit() {return "0123456789ABCDEF";}
		enum
		{
			//! the nth power of b
			n = 16,
		};
	};





} // namespace Base
} // namespace Math
} // namespace Yuni


#endif // __YUNI_CORE_MATH_BASE_H__
