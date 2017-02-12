#ifndef __YUNI_CORE_MATH_RANDOM_DEFAULT_H__
# define __YUNI_CORE_MATH_RANDOM_DEFAULT_H__

# include "distribution.h"


namespace Yuni
{
namespace Math
{
namespace Random
{



	/*!
	** \brief Constant random number generator
	**
	** Provides always the same value, given through the constructor
	*/
	template<typename T>
	class Constant : public Distribution<T, Constant>
	{
	public:
		// Name of the distribution
		static const char* Name() {return "Constant";}

	public:
		Constant(const T v) :pValue(v) {}
		~Constant() {}

		void reset() {}

		const Value next() const {return pValue;}

	private:
		//! The constant value
		const T pValue;

	}; // class Constant



} // namespace Random
} // namespace Math
} // namespace Yuni

# include "default.hxx"

#endif // __YUNI_CORE_MATH_RANDOM_RANDOM_H__
