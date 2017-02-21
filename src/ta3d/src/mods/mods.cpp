#include "mods.h"
#include <network/netclient.h>
#include <misc/resources.h>
#include <misc/paths.h>

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

		QStringList mod_list;
		TA3D::Resources::GlobDirs(mod_list,"mods/*");
        std::sort(mod_list.begin(), mod_list.end());
        mod_list.erase(std::unique(mod_list.begin(), mod_list.end()), mod_list.end());
        for (const QString &i : mod_list)
		{
            QString namae( TA3D::Paths::ExtractFileName(i) );
			if ( namae == ".." || namae == "." )	// Have to exclude both .. & .
				continue;							// because of windows finding . as something interesting
            if (Paths::Exists(Paths::Resources + "mods/" + namae + "/info.mod"))
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

		modList.push_back(ModInfo(-1, "default", "1.0", "", "This is the default set of resources, loading it unloads all mods.", ""));
		modList.back().setInstalled(true);

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

	QStringList Mods::getModNameList(const ModType type)
	{
		QStringList modList;

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
