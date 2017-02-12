
#include <yuni/yuni.h>
#include <yuni/core/event.h>
#include <iostream>


class Foo : public Yuni::IEventObserver<Foo>
{
public:
	Foo(int n)
	{
		pIndex = n;
	}

	virtual ~Foo()
	{
		destroyBoundEvents();
	}

	void bar(int v)
	{
		std::cout << "Object Index: " << pIndex << ", parameter = " << v << "\n";
	}

private:
	int pIndex;
};



int main()
{
	Yuni::Event<void (int)> event;
	Foo foo1(1);
	Foo foo2(2);
	event.connect(&foo1, &Foo::bar);

	event(42);
	event.clear();
	event(47); // Should display nothing
	event.clear();

	event.connect(&foo2, &Foo::bar);
	std::cout << "Connected to " << event.size() << " events\n";

	event(49);

	event.connect(&foo1, &Foo::bar);
	std::cout << "Connected to " << event.size() << " events\n";

	event(49);
	return 0;
}

