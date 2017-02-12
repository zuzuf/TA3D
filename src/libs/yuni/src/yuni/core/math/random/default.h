#ifndef __YUNI_CORE_MATH_RANDOM_DEFAULT_H__
# define __YUNI_CORE_MATH_RANDOM_DEFAULT_H__

# include <stdlib.h>
# include <time.h>
# include "distribution.h"


namespace Yuni
{
namespace Math
{
namespace Random
{



	/*!
	** \brief Default Pseudo random number generator
	**
	** This random number generator is the one provided by your Operating System.
	*/
	class Default : public ADistribution<int, Default>
	{
	public:
		// Name of the distribution
		static const char* Name() {return "Pseudo random numbers";}

		//! Type of a single random number
		typedef int Value;

	public:
		Default() {}
		~Default() {}

		void reset();
		void reset(const unsigned int seed);

		Value next();

		static Value min();
		static Value max();

		template<class U> Default& operator >> (U& u);
	};





} // namespace Random
} // namespace Math
} // namespace Yuni

# include "default.hxx"

#endif // __YUNI_CORE_MATH_RANDOM_RANDOM_H__
