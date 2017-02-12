#ifndef __YUNI_CORE_VALIDATOR_VALIDATOR_H__
# define __YUNI_CORE_VALIDATOR_VALIDATOR_H__

# include "../../../yuni.h"
# include "../../string.h"
# include "../validator.h"



namespace Yuni
{
namespace Validator
{
namespace Text
{



	template<class D>
	class IValidatorTmpl
	{
	public:
		/*!
		** \brief Perform a validation on any type of string
		**
		** \param u An arbitrary string
		** \return True if valid, false otherwise
		*/
		template<class U> bool validate(const U& u) const
		{
			return static_cast<const D*>(this)->validate(u);
		}

		/*!
		** \see validate()
		*/
		template<class U> bool operator () (const U& u) const
		{
			return static_cast<const D*>(this)->validate(u);
		}

	}; // class IValidatorTmpl





} // namespace Text
} // namespace Validator
} // namespace Yuni

# include "default.h"

#endif // __YUNI_CORE_VALIDATOR_VALIDATOR_H__
