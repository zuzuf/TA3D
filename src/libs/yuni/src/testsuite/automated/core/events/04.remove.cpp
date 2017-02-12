
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
	event.connect(&foo2, &Foo::bar);
	event.connect(&foo1, &Foo::bar);
	std::cout << "Empty: " << event.empty() << ", not empty: " << event.notEmpty()
		<< ", Connected to " << event.size() << " events\n";

	event(49);

	event.remove(&foo2);
	std::cout << "Empty: " << event.empty() << ", not empty: " << event.notEmpty()
		<< ", Connected to " << event.size() << " events\n";

	event(47);

	return 0;
}

