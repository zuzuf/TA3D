#ifndef __YUNI_ALGORITHM_LUHN_H__
# define __YUNI_ALGORITHM_LUHN_H__

# include "../yuni.h"
# include "../core/string/string.h"


namespace Yuni
{
namespace Algorithm
{


	/*!
	** \brief The Luhn algorithms (or Luhn formula)
	**
	** The Luhn algorithm is a simple checksum formula used to validate a variety of
	** identification numbers, such as credit card numbers.
	** It was created by IBM scientist Hans Peter Luhn and described in U.S. Patent
	** 2,950,048, filed on January 6, 1954, and granted on August 23, 1960.
	**
	** it was designed to protect against accidental errors, not malicious attacks
	** and it is a simple method for distinguishing valid numbers from collections
	** of random digits.
	*/
	class Luhn
	{
	public:
		/*!
		** \brief Checks whether a string of digits is a valid credit card number
		**
		** This method uses the `Luhn Mod 10` algorithm.
		** \code
		** // A valid credit card number
		** std::cout << Yuni::Algorithm::Luhn::IsValidCreditCardNumber("49927398716") << std::endl;
		** \endcode
		**
		** \param s A string with an arbitrary length (> 1, returns false otherwise)
		** \return True if the string is a valid credit card number, False otherwise
		*/
		static bool IsValidCreditCardNumber(const String& s);

	public:
		/*!
		** \brief Generate the check digit of a string using the `Luhn mode 10` algorithm
		**
		** \param s A string with an arbitrary length (> 1, error otherwise)
		** \return The check digit, -1 if an error has occured
		*/
		static int Mod10(const String& s);

		/*!
		** \brief Generate the check character of a string using the `Luhn mode N` algorithm
		**
		** A mapping between valid input characters and code-points is needed, to
		** specify what characters are valid, and the numerical order of those characters.
		**
		** \code
		** static int CodePointsHexa(const char c)
		** {
		** 	switch (c)
		** 	{
		** 		case '0' : return 0;
		** 		case '1' : return 1;
		** 		case '2' : return 2;
		** 		case '3' : return 3;
		** 		case '4' : return 4;
		** 		case '5' : return 5;
		** 		case '6' : return 6;
		** 		case '7' : return 7;
		** 		case '8' : return 8;
		** 		case '8' : return 8;
		** 		case '8' : return 8;
		** 	}
		** }
		**
		** int main(void)
		** {
		** 	char check = Yuni::Algorithms::Luhn::ModN("a340bd2f", &CodePointsHexa);	
		** 	std::
		** 	return 0;
		** }
		** \endcode
		**
		** \param s A string with an arbitrary length (> 1, error otherwise)
		** \return The check digit, -1 if an error has occured
		*/
		//! TODO Need implementation for `Luhn Mod N`
		//		template<typename T = String::Char, int N = 40>
		//		static int ModN(const StringBase<T,N>& s, int (*codepoint)(const StringBase<T,N>::Char));

	}; // class Luhn





} // namespace Algorithm
} // namespace Yuni

# include "luhn.hxx"

#endif // __YUNI_ALGORITHM_LUHN_H__
