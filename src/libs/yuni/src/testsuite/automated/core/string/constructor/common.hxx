
# include <yuni/test/test.h>


# define PRINT(X,A) \
	std::cout << X << " : `" << A << "`" << std::endl


struct StringTest
{
	typedef Yuni::CustomString<>  TestString;

	static void Simple()
	{
		Yuni::Test::Checkpoint checkpoint("Default constructor");
		TestString a;
		PRINT("Empty string", a);
	}

	static void FromCStar()
	{
		{
			Yuni::Test::Checkpoint checkpoint("Constructor with a C[]");
			TestString a("Hello world !");
			PRINT("From C[]", a);
		}
		{
			Yuni::Test::Checkpoint checkpoint("Constructor with a C*");
			const char* cstr = "Hello world !";
			TestString b(cstr);
			PRINT("From C*", b);
		}
	}
	
	static void Copy()
	{
		Yuni::Test::Checkpoint checkpoint("Copy Constructor");
		TestString a("Something to copy");
		TestString b(a);
		PRINT("Copy constructor", b);
	}

	template<typename U>
	static void From(const char* info, U u)
	{
		Yuni::Test::Checkpoint checkpoint(info);
		TestString a(u);
		PRINT(info, a);
	}


	static void Run()
	{
		Simple();
		FromCStar();
		Copy();
		From<std::string>("Constructor from std::string", std::string("An STL std::string"));
		From<int>("Constructor from int", -42);
		From<unsigned int>("Constructor from unsigned int", 4294967286U);
		From<float>("Constructor from float", 12.2f);
		From<double>("Constructor from double", 42.9);
	}

}; // class StringTest



