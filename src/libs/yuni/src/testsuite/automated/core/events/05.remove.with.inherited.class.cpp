
#include <yuni/yuni.h>
#include <yuni/core/event.h>
#include <iostream>


class Base : public Yuni::IEventObserver<Base>
{
public:
	Base(int n)
	{
		pIndex = n;
	}

	virtual ~Base()
	{
	}

	virtual void bar(int v)
	{
		std::cout << "[!!!base-class!!!] Object Index: " << pIndex << ", parameter = " << v << "\n";
	}

protected:
	int pIndex;
};



class Foo : public Base
{
public:
	Foo(int n)
		:Base(n)
	{
	}

	virtual ~Foo()
	{
		destroyBoundEvents();
	}

	virtual void bar(int v)
	{
		std::cout << "Object Index: " << pIndex << ", parameter = " << v << "\n";
	}

};



int main()
{
	Yuni::Event<void (int)> event;
	Foo* foo1 = new Foo(1);
	Foo* foo2 = new Foo(2);

	{
		event.connect(foo2, &Foo::bar);
		event.connect(foo1, &Foo::bar);
		std::cout << "Empty: " << event.empty() << ", not empty: " << event.notEmpty()
			<< ", Connected to " << event.size() << " events\n";

		event(49);

		event.remove(foo2);
		std::cout << "Empty: " << event.empty() << ", not empty: " << event.notEmpty()
			<< ", Connected to " << event.size() << " events\n";

		event(47);
	}


	std::cout << "\n---\n\n";
	{
		event.clear();
		event.connect(foo2, &Foo::bar);
		event.connect(foo1, &Foo::bar);

		Base* base = foo2;
		event.remove(base);
		std::cout << "Empty: " << event.empty() << ", not empty: " << event.notEmpty()
			<< ", Connected to " << event.size() << " events\n";
		event(42);
	}

	
	std::cout << "\n---\n\n";
	{
		event.clear();
		Base* b1 = foo1;
		Base* b2 = foo2;
		event.connect(b2, &Base::bar);
		event.connect(b1, &Base::bar);

		Base* base = foo2;
		event.remove(base);
		std::cout << "Empty: " << event.empty() << ", not empty: " << event.notEmpty()
			<< ", Connected to " << event.size() << " events\n";
		event(42);
	}



	delete foo1;
	delete foo2;
	return 0;
}

