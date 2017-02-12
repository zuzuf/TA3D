#include <iostream>
#include <list>
#include <cstdlib>
#include <ctime>
#include <yuni/core/smallobject.h>

using namespace Yuni;


// Define 2 classes generating objects with same size, one who inherits Yuni::SmallObject, and one who does not.

struct MyObject
{
	MyObject(int i1, char c1, double d1): i(i1), c(c1), d(d1)
	{}

	int i;
	char c;
	double d;
};

struct MySmallObject: public SmallObject<MySmallObject>
{
	MySmallObject(int i1, char c1, double d1): i(i1), c(c1), d(d1)
	{}

	int i;
	char c;
	double d;
};



// Add / Removal functions

template<typename T>
void add(std::list<T*>& tab)
{
	tab.push_back(new T(0, '\0', 0.0f));
}

template<typename T>
void removeFront(std::list<T*>& tab)
{
	if (tab.empty())
		return;
	T* obj = tab.front();
	tab.pop_front();
	delete obj;
}

template<typename T>
void removeBack(std::list<T*>& tab)
{
	if (tab.empty())
		return;
	T* obj = tab.back();
	tab.pop_back();
	delete obj;
}




// Allocation / Deallocation patterns

template<typename T>
void bulkAlloc(unsigned int nb, std::list<T*>& tab)
{
	for (unsigned int i = 0; i < nb; ++i)
		add(tab);
}

template<typename T>
void sameOrderDealloc(unsigned int nb, std::list<T*>& tab)
{
	while (--nb)
		removeFront(tab);
}

template<typename T>
void reverseDealloc(unsigned int nb, std::list<T*>& tab)
{
	while (--nb)
		removeBack(tab);
}

// In this case, nb is the total amount of alloc + dealloc done.
template<typename T>
void butterflyAllocDealloc(unsigned int nb, std::list<T*>& tab)
{
	while (--nb)
	{
		if (rand() % 2)
			add(tab);
		else
		{
			if (rand() % 2)
				removeFront(tab);
			else
				removeBack(tab);
		}
	}
}

template<typename T>
void testAllocator()
{
	// Fix a number of allocations to do
	const unsigned int nbAlloc = 10000000;
	// List to store our allocated objects
	std::list<T*> tab;

	time_t start = time(NULL);
	time_t old = start;

	// Bulk allocation consists of many allocations at a time
	std::cout << "Bulk allocation: ";
	bulkAlloc(nbAlloc, tab);
	time_t now = time(NULL);
	std::cout << now - old << " seconds" << std::endl;
	old = now;

	// Butterfly allocation means many allocations/deallocations in any order
	std::cout << "Butterfly allocation: ";
	butterflyAllocDealloc(nbAlloc, tab);
	now = time(NULL);
	std::cout << now - old << " seconds" << std::endl;
	old = now;

	// Deallocation simulating objects being freed in the order
	// they were allocated
	std::cout << "Same order deallocation: ";
	sameOrderDealloc(tab.size() / 2, tab);
	now = time(NULL);
	std::cout << now - old << " seconds" << std::endl;
	old = now;

	// Deallocation simulating objects being freed in the reverse order
	// to which they were allocated
	std::cout << "Reverse order deallocation: ";
	reverseDealloc(tab.size(), tab);
	now = time(NULL);
	std::cout << now - old << " seconds" << std::endl;

	std::cout << "Total time: " << now - start << " seconds" << std::endl;
}




int main(void)
{
	// Initialize random seed
	srand((unsigned int)time(NULL));

	std::cout << "Begin test with default allocator..." << std::endl;
	// Test the algorithm with default allocator
	testAllocator<MyObject>();

	std::cout << std::endl;

	std::cout << "Begin test with Yuni's allocator..." << std::endl;
	// Test the algorithm with Yuni's allocator
	testAllocator<MySmallObject>();

	return 0;
}
