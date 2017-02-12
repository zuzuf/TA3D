#include <iostream>
#include <string>
#include <complex>
#include <yuni/core/string.h>
#include <yuni/core/variant.h>

using namespace Yuni;
using namespace std;

int main(void)
{
	std::cout << "int 12       -> int     : " << Variant(int(12)).to<int>() << std::endl;
	std::cout << "int 12       -> float   : " << Variant(int(12)).to<float>() << std::endl;
	std::cout << "int 12       -> double  : " << Variant(int(12)).to<double>() << std::endl;
	std::cout << "int 12       -> string  : " << Variant(int(12)).to<String>() << std::endl;

	std::cout << "float 42.7f  -> int     : " << Variant(42.7f).to<int>() << std::endl;
	std::cout << "float 42.7f  -> float   : " << Variant(42.7f).to<float>() << std::endl;
	std::cout << "float 42.7f  -> double  : " << Variant(42.7f).to<double>() << std::endl;
	std::cout << "float 42.7f  -> string  : " << Variant(42.7f).to<String>() << std::endl;

	std::cout << "str '98.4 p' -> int     : " << Variant("98.4 p").to<int>() << std::endl;
	std::cout << "str '98.4 p' -> float   : " << Variant("98.4 p").to<float>() << std::endl;
	std::cout << "str '98.4 p' -> double  : " << Variant("98.4 p").to<double>() << std::endl;
	std::cout << "str '98.4 p' -> string  : " << Variant("98.4 p").to<String>() << std::endl;

	std::cout << "true         -> string  : " << Variant(true).to<String>() << std::endl;
	std::cout << "false        -> string  : " << Variant(false).to<String>() << std::endl;
	std::cout << "true         -> float   : " << Variant(true).to<float>() << std::endl;
	std::cout << "false        -> float   : " << Variant(false).to<float>() << std::endl;

	std::cout << "str 'On'     -> bool    : " << Variant("On").to<bool>() << std::endl;
	std::cout << "str 'Off'    -> bool    : " << Variant("Off").to<bool>() << std::endl;

	return 0;
}
