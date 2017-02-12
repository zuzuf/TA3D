#ifndef __YUNI_CORE_PREDICATE_RESULT_AND_H__
# define __YUNI_CORE_PREDICATE_RESULT_AND_H__


namespace Yuni
{
namespace Result
{


	template<typename T>
	struct And
	{
	public:
		//! Type of the result
		typedef bool ResultType;

	public:
		//! \name Constructors
		//@{
		/*!
		** \brief Default Constructor
		*/
		And()
			:pValue(true)
		{}
		And(bool defaultValue)
			:pValue(defaultValue)
		{}
		/*!
		** \brief Copy constructor
		*/
		And(const And& rhs) :pValue(rhs.pValue) {}
		//@}

		//! Take into account a new value
		void operator () (const T& v) { pValue = (pValue && v);}

		/*!
		** \brief The Final result
		*/
		ResultType result() const {return pValue;}

		/*!
		** \brief Reset the internal counter
		*/
		void reset() {pValue = true;}
		void reset(bool v) {pValue = v;}

	private:
		//! The internal counter
		bool pValue;

	}; // class And




} // namespace Result
} // namespace Yuni

#endif // __YUNI_CORE_PREDICATE_RESULT_AND_H__
