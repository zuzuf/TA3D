#ifndef __TA3D_MODS_MODS_H__
#define __TA3D_MODS_MODS_H__

#include "modinfo.h"
#include <threads/thread.h>

namespace TA3D
{
	class Mods : public ObjectSync
	{
	public:
		enum ModType { MOD_ALL, MOD_INSTALLED, MOD_INSTALLABLE };

		Mods();
		~Mods();
		void update();
		void save();
		ModInfo::List getModList( const ModType type = MOD_ALL );
		QStringList getModNameList( const ModType type = MOD_ALL );
		ModInfo *getMod(int ID);
	private:
		ModInfo::List mods;
	public:
		static Mods *instance();
	};
}

#endif
