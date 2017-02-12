
#include <iostream>
#include <yuni/yuni.h>
#include <yuni/core/math/random/default.h>
#include <yuni/core/math/random/range.h>



/*!
** \brief Simple class to test a random number generator
*/
template<class Generator>
struct Test
{
	static void Run()
	{
		Generator generator;

		// Initializing the random number generator
		generator.reset();

		// Information about the generator
		std::cout << " * Distribution    : " << generator.name() << std::endl;
		std::cout << "   Range           : " << generator.min() << " .. " << generator.max() << std::endl;

		// Generating a few random numbers
		std::cout << "   A random number : " << generator() << std::endl;
		std::cout << "   Another one     : " << generator() << std::endl;

		std::cout << std::endl;
	}

}; // struct Test




int main(void)
{
	// The distribution used in our example
	typedef Yuni::Math::Random::Default   Distribution;

	Test< Yuni::Math::Random::Range<Distribution,  0,    1, float> >::Run();
	Test< Yuni::Math::Random::Range<Distribution, -1,    0, float> >::Run();
	Test< Yuni::Math::Random::Range<Distribution, 10,  140, float> >::Run();
	Test< Yuni::Math::Random::Range<Distribution,  0, 1000, int> >::Run();
	return 0;
}
