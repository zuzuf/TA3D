
#include <yuni/yuni.h>
#include <yuni/core/system/environment.h>
#include <iostream>
#include <string>


using namespace Yuni;


template<class AnyStringT> void SimpleTest(const char* msg)
{
	AnyStringT s = "HOME";
	std::cout << msg << " : " << System::Environment::Read(s) << std::endl;
}

template<class AnyStringT1, class AnyStringT2> void ValueByReference(const char* msg)
{
	AnyStringT1 key = "HOME";
	AnyStringT2 value;
	if (System::Environment::Read(key, value))
	{
		std::cout << msg << " : " << value << std::endl;
	}
	else
		std::cout << msg << " : failed\n";
}


int main(void)
{
	std::cout << "Simple tests:\n";
	SimpleTest<const char*>("const char*");
	SimpleTest<char*>("char*");
	SimpleTest<String>("yuni::string");
	SimpleTest<std::string>("std::string");

	std::cout << "\nWith a given reference:\n";
	ValueByReference<const char*, String>("const char*/Yuni::String");
	ValueByReference<const char*, std::string>("const char*/std::string");
	ValueByReference<char*, String>("char*/Yuni::String");
	ValueByReference<char*, std::string>("char*/std::string");
	ValueByReference<String, String>("Yuni::String/Yuni::String");
	ValueByReference<String, std::string>("Yuni::String/std::string");
	ValueByReference<std::string, String>("std::string/Yuni::String");
	ValueByReference<std::string, std::string>("std::string/std::string");

	return 0;
}
