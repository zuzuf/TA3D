
#include <iostream>
#include <yuni/core/singleton/singleton.h>

class MyManager: public Yuni::Singleton<MyManager>
{
public:
	typedef Yuni::Singleton<MyManager> Singleton;

public:
	void doSomething()
	{
		std::cout << "Ping" << std::endl;
	}

	// In the simplest sample, the constructor is public
	// In more advanced samples, we prove that it can be made private for more safety,
	// if the correct friend declaration is added.
	MyManager() {}

private:
	// Hide the rest of the constructors and assignment operators
	MyManager(const MyManager&);
	MyManager& operator = (const MyManager&);
};


int main(void)
{
	MyManager& managerInstance = MyManager::Singleton::Instance();

	managerInstance.doSomething();
}
