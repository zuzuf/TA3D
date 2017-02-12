#ifndef __YUNI_CORE_PREDICATE_RESULT_OR_H__
# define __YUNI_CORE_PREDICATE_RESULT_OR_H__


namespace Yuni
{
namespace Result
{


	template<typename T>
	struct Or
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
		Or()
			:pValue(false)
		{}
		Or(bool defaultValue)
			:pValue(defaultValue)
		{}
		/*!
		** \brief Copy constructor
		*/
		Or(const Or& rhs) :pValue(rhs.pValue) {}
		//@}

		//! Take into account a new value
		void operator () (const T& v) { pValue = (pValue || v);}

		/*!
		** \brief The Final result
		*/
		ResultType result() const {return pValue;}

		/*!
		** \brief Reset the internal counter
		*/
		void reset() {pValue = false;}
		void reset(bool v) {pValue = v;}

	private:
		//! The internal counter
		bool pValue;

	}; // class Or




} // namespace Result
} // namespace Yuni

#endif // __YUNI_CORE_PREDICATE_RESULT_OR_H__
