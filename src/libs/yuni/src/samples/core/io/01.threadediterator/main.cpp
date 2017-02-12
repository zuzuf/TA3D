
#include <yuni/yuni.h>
#include <yuni/core/io/directory/iterator.h>
#include <iostream>

using namespace Yuni;



class MyIterator : public Core::IO::Directory::IIterator<true>
{
public:
	//! Flow
	typedef Core::IO::Flow Flow;

public:
	MyIterator() {}
	virtual ~MyIterator()
	{
		// For code robustness and to avoid corrupt vtable
		stop();
	}

protected:
	virtual bool onStart(const String& rootFolder)
	{
		std::cout << " [+] " << rootFolder << std::endl;
		pCounter = 1;
		pFileCount = 0;
		pFolderCount = 0;
		pTotalSize = 0;
		return true;
	}

	virtual Flow onBeginFolder(const String&, const String&, const String& name)
	{
		printSpaces();
		std::cout << " [+] " << name << std::endl;
		++pCounter;
		++pFolderCount;
		return Core::IO::flowContinue;
	}

	virtual void onEndFolder(const String&, const String&, const String&)
	{
		--pCounter;
	}

	virtual Flow onFile(const String&, const String&, const String& name, uint64 size)
	{
		printSpaces();
		std::cout << "  -  " << name << " (" << size << " bytes)" << std::endl;
		++pFileCount;
		pTotalSize += size;
		return Core::IO::flowContinue;
	}

	virtual void onTerminate()
	{
		std::cout << "\n";
		std::cout << pFolderCount << " folder(s), " << pFileCount << " file(s),  "
			<< pTotalSize << " bytes" << std::endl;
	}

private:
	void printSpaces()
	{
		for (unsigned int i = 0; i != pCounter; ++i)
			std::cout << "    ";
	}

private:
	unsigned int pCounter;
	unsigned int pFolderCount;
	unsigned int pFileCount;
	size_t pTotalSize;
};


int main(int argc, char** argv)
{
	if (argc <= 1)
	{
		std::cout << "usage: " << argv[0] << " [directory0] [directory1]...\n";
		return 0;
	}
	MyIterator iterator;

	for (int i = 1; i < argc; ++i)
		iterator.add(argv[i]);
	iterator.start();
	iterator.wait();
	return 0;
}

