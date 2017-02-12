
#include <yuni/yuni.h>
#include <yuni/core/event.h>
#include <iostream>


static void foo(int v)
{
	std::cout << "Value: " << v << "\n";
}



int main()
{
	Yuni::Event<void (int)> event;
	event.connect(&foo);
	event(42);
	return 0;
}

