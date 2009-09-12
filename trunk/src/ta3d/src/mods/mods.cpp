#include "mods.h"
#include "../network/netclient.h"
#include "../misc/resources.h"
#include "../misc/paths.h"

namespace TA3D
{
	Mods *Mods::instance()
	{
		static Mods sInstance;
		return &sInstance;
	}

	Mods::Mods()
	{
		update();
	}

	Mods::~Mods()
	{
		save();
	}

	void Mods::update()
	{
		lock();

		mods.clear();

		String::List mod_list;
		TA3D::Resources::GlobDirs(mod_list,"mods/*");
		mod_list.sort();
		mod_list.unique();
		for (String::List::iterator i = mod_list.begin() ; i != mod_list.end() ; ++i)
		{
			String namae( TA3D::Paths::ExtractFileName(*i) );
			if ( namae == ".." || namae == "." )	// Have to exclude both .. & .
				continue;							// because of windows finding . as something interesting
			if (Paths::Exists(Paths::Resources + "mods" + Paths::Separator + namae + Paths::Separator + "info.mod"))
			{
				ModInfo mod;
				mod.read(namae);
				mods.push_back(mod);
			}
		}

		ModInfo::List netclientList = NetClient::instance()->getModList();

		for(ModInfo::List::iterator i = netclientList.begin() ; i != netclientList.end() ; ++i)
		{
			bool found = false;
			for(ModInfo::List::iterator e = mods.begin() ; e != mods.end() && !found ; ++e)
			{
				if (e->getID() == i->getID())
				{
					found = true;
					if (e->isInstalled() && e->getVersion() != i->getVersion())
						e->setUpdateAvailable(true);
					else if (!e->isInstalled())
						*e = *i;
				}
			}
			if (!found)
				mods.push_back(*i);
		}

		save();

		unlock();
	}

	void Mods::save()
	{
		lock();

		for(ModInfo::List::iterator it = mods.begin() ; it != mods.end() ; ++it)
			it->write();

		unlock();
	}

	ModInfo::List Mods::getModList(const ModType type)
	{
		ModInfo::List modList;

		lock();

		for(ModInfo::List::iterator i = mods.begin() ; i != mods.end() ; ++i)
		{
			if (type == MOD_ALL
				|| (type == MOD_INSTALLED && i->isInstalled())
				|| (type == MOD_INSTALLABLE && !i->isInstalled()))
				modList.push_back(*i);
		}

		unlock();

		return modList;
	}

	String::List Mods::getModNameList(const ModType type)
	{
		String::List modList;

		lock();

		for(ModInfo::List::iterator i = mods.begin() ; i != mods.end() ; ++i)
		{
			if (type == MOD_ALL
				|| (type == MOD_INSTALLED && i->isInstalled())
				|| (type == MOD_INSTALLABLE && !i->isInstalled()))
				modList.push_back(i->getName());
		}

		unlock();

		return modList;
	}

	ModInfo *Mods::getMod(int ID)
	{
		MutexLocker mLock(pMutex);
		for(ModInfo::List::iterator i = mods.begin() ; i != mods.end() ; ++i)
			if (i->getID() == ID)
				return &(*i);

		return NULL;
	}
}
