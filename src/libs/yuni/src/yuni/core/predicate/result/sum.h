#ifndef __YUNI_CORE_PREDICATE_RESULT_SUM_H__
# define __YUNI_CORE_PREDICATE_RESULT_SUM_H__


namespace Yuni
{
namespace Result
{


	template<typename T>
	struct Sum
	{
	public:
		//! Type of the result
		typedef T ResultType;

	public:
		//! \name Constructors
		//@{
		/*!
		** \brief Default Constructor
		*/
		Sum() :pValue() {}
		/*!
		** \brief Copy constructor
		*/
		Sum(const Sum& rhs) :pValue(rhs.pValue) {}
		//@}

		//! Take into account a new value
		void operator () (const T& v) { pValue += v;}

		/*!
		** \brief The Final result
		*/
		ResultType result() const {return pValue;}

		/*!
		** \brief Reset the internal counter
		*/
		void reset() {pValue = T();}

	private:
		//! The internal counter
		T pValue;

	}; // class Sum




} // namespace Result
} // namespace Yuni

#endif // __YUNI_CORE_PREDICATE_RESULT_SUM_H__
