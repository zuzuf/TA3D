
#include "test.hxx"

int main(void)
{
	Dummy<Yuni::Policy::ClassLevelLockable> dummy;
	dummy.runWithLock();
	return 0;
}
