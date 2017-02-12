
#include <iostream>
#include <yuni/yuni.h>
#include <yuni/core/math/random/default.h>
#include <yuni/core/math/random/table.h>


int main(void)
{
	// This example uses the default random number generator, the one provided
	// by the Operating system (pseudo random numbers)
	Yuni::Math::Random::Table<Yuni::Math::Random::Default>  generator;

	// The random number generator *should* be initialized.
	// When using a table, the random number generator will be initialized then
	// the cache will be filled with a new set of numbers.
	// To generate a new set of numbers without resetting the generator, you should
	// use `fill()` instead.
	generator.reset();

	// Information about the generator
	std::cout << "Distribution    : " << generator.name() << std::endl;
	std::cout << "Cache size      : " << generator.size() << std::endl;
	std::cout << "Range           : " << generator.min() << " .. " << generator.max() << std::endl;

	// Generating a few random numbers
	std::cout << "A random number : " << generator() << std::endl;
	std::cout << "Another one     : " << generator() << std::endl;
	return 0;
}
