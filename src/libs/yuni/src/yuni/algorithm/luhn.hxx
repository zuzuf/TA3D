#ifndef __YUNI_ALGORITHM_LUHN_HXX__
# define __YUNI_ALGORITHM_LUHN_HXX__


namespace Yuni
{
namespace Algorithm
{


	inline bool Luhn::IsValidCreditCardNumber(const String& s)
	{
		return (Mod10(s) == 0);
	}



} // namespace Algorithm
} // namespace Yuni


#endif // __YUNI_ALGORITHM_LUHN_HXX__
