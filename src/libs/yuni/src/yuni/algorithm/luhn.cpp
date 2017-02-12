
#include "luhn.h"
#include "../core/string.h"


namespace Yuni
{
namespace Algorithm
{

	int Luhn::Mod10(const String& s)
	{
		// The string must have at least one char
		if (s.size() > 1)
		{
			// The algorithm :
			// 1 - Counting from the check digit, which is the rightmost, and moving
			//     left, double the value of every second digit.
			// 2 - Sum the digits of the products together with the undoubled digits
			//     from the original number.
			// 3 - If the total ends in 0 (put another way, if the total modulo 10 is
			//     congruent to 0), then the number is valid according to the Luhn formula
			//
			static const int prefetch[] = {0, 2, 4, 6, 8, 1, 3, 5, 7, 9};

			int n;
			int sum = 0;
			bool alternate = true;

			// For each char
			for (const String::Char* c = s.c_str(); *c != '\0'; ++c)
			{
				// Each char in the string must be a digit
				if (!String::IsDigit(*c))
					return false;
				// The `real` digit
				n = *c - '0';
				// Computing the sum
				sum += (alternate = !alternate) ? prefetch[n] : n;
			}
			return sum % 10;
		}
		return -1;
	}



} // namespace Algorithm
} // namespace Yuni


