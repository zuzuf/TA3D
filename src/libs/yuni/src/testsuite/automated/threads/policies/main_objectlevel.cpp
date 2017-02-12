
#include "test.hxx"

int main(void)
{
	Dummy<Yuni::Policy::ObjectLevelLockable> dummy;
	dummy.runWithLock();
	return 0;
}
