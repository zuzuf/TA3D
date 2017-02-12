
#include <yuni/yuni.h>
#include <yuni/core/event.h>
#include <iostream>


class Foo : public Yuni::IEventObserver<Foo>
{
public:
	virtual ~Foo()
	{
		destroyBoundEvents();
	}

	void bar(int v)
	{
		std::cout << "Value: " << v << "\n";
	}
};



int main()
{
	Yuni::Event<void (int)> event;
	Foo foo;
	event.connect(&foo, &Foo::bar);
	event(42);
	return 0;
}

