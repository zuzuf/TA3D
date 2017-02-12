
#include <iostream>
#include <yuni/script/script.h>
#include <yuni/script/lua/lua.h>

/*
** This sample showcases the Script module.
**
** It shows how to use Lua functions from a C++ application, and
** conversely.
**/

// We will first define a standard C++ function, to be called from Lua.
namespace
{
	Yuni::String sampleFunction(int arg1, double arg2, Yuni::String arg3, void* arg4)
	{
		std::cout << "[ C++] Now in sampleFunction(" 
			<< arg1 << ", "
			<< arg2 << ", "
			<< arg3 << ", "
			<< arg4 << ")"
			<< std::endl;
	 	return "String from sampleFunction()";
	}
}

// We also define a custom object, to call a method of this object from Lua.
class SampleObject
{
public:
	int sampleMethod(const Yuni::String& argument)
	{
		std::cout << "[ C++] Now in SampleObject::sampleMethod(" << argument << ")\n";
		return 42;
	}
};


int main()
{

	// Create a Lua script object, to execute our demo script.
	// We store it within an abstract script interface, because every standard
	// function is accessible through it.
	// This means you can mix script languages in your code without changing types.
	Yuni::Script::AScript* sc = new Yuni::Script::Lua();

	// Load a hello world script from a file
	if (!sc->loadFromFile("helloworld.lua"))
	{
		// An error has occured.
		std::cout << "[ C++] Error while loading script from file." << std::endl;
		exit(1);
	}


	// Note: at this point, depending on the script engine, the script may or may
	// not have been processed by the script engine. Still, it is recorded, and every
	// subsequent action will be executed in order.


	// Also load a block of code. This code will be executed as if it was loaded from a file.
	// You can include dynamically generated code this way, in this case a random array.
	Yuni::String luaArray("sampleArray = { ");
	for (int i = 0; i < 15; ++i)
		luaArray.appendFormat("%f%c", (rand() % 1000) * 0.001, (i < 99 ? ',' : ' '));
	luaArray += "};";
	if (!sc->appendFromString(luaArray))
	{
		// An error has occured.
		std::cout << "[ C++] Error while loading string script." << std::endl;
		exit(1);
	}

	// We call prepare to ensure that all the code blocks we loaded previously are parsed
	// and executed, so we are ready to continue.
	sc->prepare();

	// Ask lua to print our random array via a function inside helloworld.lua.
	sc->call(NULL, "printRandomArray");

	// Perform a second function call, this time with arguments.
	{
		// Declare a variant to store the call results
		Yuni::Any ret;

		// Then call a function with one argument, for example an Int.
		if (!sc->call(&ret, "sampleLuaFunction", "Hello, world", 15.2))
		{
			std::cout << "[ C++] Error while calling sampleLuaFunction(\"Hello, world\", 15.2)" << std::endl;
			exit(1);
		}

		std::cout << "[ C++] Got result of type " << ret.type().name() << std::endl;
	}


	// Last, we try to bind a method of a C++ object and a C++ function,
	// then call them from Lua.

	SampleObject sampleInst;
	if (!sc->bind("SampleObject__sampleMethod", &sampleInst, &SampleObject::sampleMethod))
	{
		std::cout << "[ C++] Error while binding SampleObject::sampleMethod()" << std::endl;
		exit(1);
	}

	if (!sc->bind("sampleFunction", &sampleFunction))
	{
		std::cout << "[ C++] Error while binding sampleFunction()" << std::endl;
		exit(1);
	}

	if (!sc->call(NULL, "callBackSampleFunctions"))
	{
		std::cout << "[ C++] Error while calling callBackSampleFunctions()" << std::endl;
		exit(1);
	}


	// Test a few numbers
	for (int i = 0 ; i < 64; ++i)
	{
		Yuni::uint64 u64 = 1UL << i;
		Yuni::sint64 s64 = ~(1UL << 6);

		std::cout << "[ C++] u64: " << u64 << std::endl;
		sc->call(NULL, "printNumber", u64);
	}
	

	// Finally, destroy the script object to release the resources.
	delete sc;

	// Goodbye !
	return 0;
}
