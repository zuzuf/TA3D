#ifndef __YUNI_CORE_MATH_RANDOM_TABLE_H__
# define __YUNI_CORE_MATH_RANDOM_TABLE_H__

# include "../math.h"
# include "../../../thread/policy.h"
# include "../../static/assert.h"
# include "../../static/if.h"
# include "../../atomic/int.h"


namespace Yuni
{
namespace Math
{
namespace Random
{


	/*!
	** \brief Pre-Cached table of random numbers
	**
	** This class behaves like any other random number generator except random numbers
	** come from a prefetch cache.
	**
	** \code
	** #include <iostream>
	** #include <yuni/core/math/random/default.h>
	** #include <yuni/core/math/random/table.h>
	**
	** using namespace Yuni;
	**
	** int main()
	** {
	** 		// A table for pre-cached random numbers
	** 		Math::Random::Table<Math::Random::Default>  randomTable;
	** 		// Generating a set pre-cached random numbers
	** 		std::cout << "Generating " << randomTable.size() << " random numbers..." << std::endl;
	** 		randomTable.reset();
	**
	** 		std::cout << "A random number : " << randomTable() << std::endl;
	** 		std::cout << "Another one     : " << randomTable() << std::endl;
	** 		return 0;
	** }
	** \endcode
	**
	** Do not forget that you have to reset the random number generator yourself,
	** As this operation may be costly, You may want to control it. The other
	** reason is that some specific parameters may be required
	**
	** Even if it is possible to make this class thread-safe (using the godd threading
	** policy, it is no really advised.  The real purpose of this class
	** is to avoid the high cost of a Random number generation (in a real-time
	** game for example) and additional locks might be costly as well.
	** In most cases you would prefer a `table` per thread.
	**
	** \tparam D Any random number generator
	** \tparam TableSize The count of random numbers to cache
	** \tparam Cyclic True to Cycle through the list or False to regenerate a new set
	** when all pre-cached random numbers are exhausted
	** \tparam TP The Threading Policy
	*/
	template<class D,                                         // The distribution
		int TableSize = 200000,                               // Number of pre-cached numbers
		bool Cyclic = true,                                   // Cycle through the list or regenerate a new set ?
		template<class> class TP = Policy::SingleThreaded     // The Threading policy
		>
	class Table
		:public TP<Table<D,TableSize, Cyclic, TP> >
		,public D
	{
	public:
		//! Type for a single random number
		typedef typename D::Value Value;
		//! The threading policy
		typedef TP<Table<D,TableSize, Cyclic, TP> > ThreadingPolicy;

		// Assert about the table size
		YUNI_STATIC_ASSERT((TableSize > 10), MathRandomTable_InvalidTableSize);

		enum
		{
			//! The table size
			tableSize = TableSize,
		};

	public:
		/*!
		** \brief Name of the distribution
		**
		** This name is given by the template parameter D.
		*/
		static const char* Name();

	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Default constructor
		**
		** The table will be filled with random numbers provided by the
		** given distribution.
		*/
		Table();
		//! Destructor
		~Table();
		//@}

		/*!
		** \brief Reset the random number generator
		**
		** The method `fill()` will be implicity called to generate a new set
		** of random numbers.
		*/
		void reset();

		/*!
		** \brief Get the number of random number in the table
		*/
		static int size();

		/*!
		** \brief Get the next random number
		*/
		const Value next();

		/*!
		** \brief Fill the table with a new set of random numbers
		*/
		void fill();

		//! The operator `>>`
		template<class U> Table& operator >> (U& u);

		//! The operator `()`
		const Value operator () ();

	private:
		void fillWL();

	private:
		//! Offset
		typedef typename Static::If<(!ThreadingPolicy::threadSafe), Atomic::Int<>, int>::Type  OffsetType;
		//! Position in the cache
		OffsetType pOffset;
		//! The cache
		typename D::Value pCache[TableSize];

	}; // class Table






} // namespace Random
} // namespace Math
} // namespace Yuni

# include "table.hxx"

#endif // __YUNI_CORE_MATH_RANDOM_TABLE_H__
