
#include <iostream>
#include <yuni/core/singleton/singleton.h>

//
// A log class for logging errors or information
// It is defined as a phoenix singleton
//
class Log: Yuni::Singleton<Log, Yuni::Policy::Creation::EmptyConstructor, Yuni::Policy::Lifetime::Phoenix>
{
public:
	typedef Yuni::Singleton<Log, Yuni::Policy::Creation::EmptyConstructor, Yuni::Policy::Lifetime::Phoenix> Singleton;

public:
	void print(std::ostream& out, const char* message)
	{
		out << "Log : ";
		out << message;
	}


protected:
	Log()
	{
		std::cout << "Log created." << std::endl;
	}
	~Log()
	{
		std::cout << "Log destroyed." << std::endl;
	}

	// Hide the rest of the constructors and assignment operators
	Log(const Log&);
	Log& operator = (const Log&);

	friend class Yuni::Policy::Creation::EmptyConstructor<Log>;
};


//
// Keyboard writes to the log on creation and destruction
//
class Keyboard : public Yuni::Singleton<Keyboard>
{
public:
	typedef Yuni::Singleton<Keyboard> Singleton;

protected:
	Keyboard()
	{
		std::cout << "Keyboard created." << std::endl;
		Log::Singleton::Instance().print(std::cout, "Keyboard creation.\n");
	}
	~Keyboard()
	{
		Log::Singleton::Instance().print(std::cout, "Keyboard destruction.\n");
		std::cout << "Keyboard destroyed." << std::endl;
	}

	// Hide the rest of the constructors and assignment operators
	Keyboard(const Keyboard&);
	Keyboard& operator = (const Keyboard&);

	friend class Yuni::Policy::Creation::EmptyConstructor<Keyboard>;
};


//
// Display only writes to the log on destruction
//
class Display: public Yuni::Singleton<Display>
{
public:
	typedef Yuni::Singleton<Display> Singleton;

protected:
	Display()
	{
		std::cout << "Display created." << std::endl;
	}
	~Display()
	{
		Log::Singleton::Instance().print(std::cout, "Display destruction.\n");
		std::cout << "Display destroyed." << std::endl;
	}

private:
	// Hide the rest of the constructors and assignment operators
	Display(const Display&);
	Display& operator = (const Display&);

	friend class Yuni::Policy::Creation::EmptyConstructor<Display>;
};


// What happens in this sample is that the Display and Keyboard both use the Log
int main(void)
{
	// On creation of Display, Log will also be created in order to log information
	Display& display = Display::Singleton::Instance();
	// Keyboard will use the already existing Log
	Keyboard& keyboard = Keyboard::Singleton::Instance();

	return 0;
}
// And all the fun stuff happens now :
// Keyboard was created last and so it is destroyed first. No problem there.
// Then Log is (more or less) destroyed.
// When finally Display is destroyed, Log is reborn from its ashes to serve its needs.
// And in the end, Log is destroyed again and all is well with the world.
