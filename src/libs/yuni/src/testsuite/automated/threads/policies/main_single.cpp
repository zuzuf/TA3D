
#include "test.hxx"

int main(void)
{
	Dummy<Yuni::Policy::SingleThreaded> dummy;
	dummy.runWithLock();
	return 0;
}
