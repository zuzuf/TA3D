#include "mods.h"

namespace TA3D
{
	Mods *Mods::instance()
	{
		static Mods sInstance;
		return &sInstance;
	}
}
