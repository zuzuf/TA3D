
# include <yuni/test/test.h>



struct StringTest
{
	typedef Yuni::CustomString<>  TestString;

	static void WithCStar()
	{
		{
			Yuni::Test::Checkpoint checkpoint("With a C[]");
			TestString a;
			a = "Hello world";
			std::cout << a << std::endl;
		}
		{
			Yuni::Test::Checkpoint checkpoint("With a C*");
			const char* cstr = "Hello world !";
			TestString a;
			a = cstr;
			std::cout << a << std::endl;
		}
	}


	static void AnotherString()
	{
		Yuni::Test::Checkpoint checkpoint("With another string");
		TestString a;
		TestString b("Dummy text");
		a = b;
		std::cout << a << std::endl;
	}

	static void StdString()
	{
		Yuni::Test::Checkpoint checkpoint("With an std::string");
		TestString a;
		std::string b("Dummy text");
		a = b;
		std::cout << a << std::endl;
	}

	static void WithInt()
	{
		Yuni::Test::Checkpoint checkpoint("With an integer");
		TestString a;
		a = 42;
		std::cout << a << std::endl;
	}

	static void WithUnsignedLong()
	{
		Yuni::Test::Checkpoint checkpoint("With an unsigned long");
		TestString a;
		a = 42ul;
		std::cout << a << std::endl;
	}

	static void WithHexadecimal()
	{
		Yuni::Test::Checkpoint checkpoint("With a hexadecimal number");
		TestString a;
		a = 0x2A;
		std::cout << a << std::endl;
	}
	
	static void Run()
	{
		WithCStar();
		AnotherString();
		StdString();
		WithInt();
		WithUnsignedLong();
		WithHexadecimal();
	}

}; // class StringTest



