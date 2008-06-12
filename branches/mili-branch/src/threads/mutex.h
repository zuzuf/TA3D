#ifndef __TA3D_THREADS_MUTEX_H__
# define __TA3D_THREADS_MUTEX_H__

#include "cCriticalSection.h"


namespace TA3D
{

class Mutex : protected cCriticalSection
{
private:
	bool pLocked;

public:
	Mutex(): pLocked(false) {CreateCS();}
	~Mutex() {DeleteCS();}
	void lock() {pLocked = true; EnterCS();}
	void unlock() {pLocked = false; LeaveCS();}
	bool isLocked() const {return pLocked;}

}; // class Mutex


} // namespace TA3D

#endif // __TA3D_THREADS_MUTEX_H__
