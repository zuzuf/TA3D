
#include <iostream>
#include <yuni/yuni.h>
#include <yuni/core/math/random/default.h>


int main(void)
{
	// This example uses the default random number generator, the one provided
	// by the Operating system (pseudo random numbers)
	Yuni::Math::Random::Default  generator;

	// The random number generator *should* be initialized. Each generator has
	// its own parameters but most of the time the default configuration should
	// fit your needs.
	generator.reset();

	// Information about the generator
	std::cout << "Distribution    : " << generator.name() << std::endl;
	std::cout << "Range           : " << generator.min() << " .. " << generator.max() << std::endl;

	// Generating a few random numbers
	std::cout << "A random number : " << generator() << std::endl;
	std::cout << "Another one     : " << generator() << std::endl;
	return 0;
}
