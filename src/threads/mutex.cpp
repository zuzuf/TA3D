#include "mutex.h"

namespace TA3D
{


    Mutex::Mutex()
    {
        pMutex = SDL_CreateMutex();
    }


	Mutex::~Mutex()
	{
	    SDL_DestroyMutex(pMutex);
	}

} // namespace TA3D
