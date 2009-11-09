#ifndef __TA3D_MODS_MODINFO_H__
# define __TA3D_MODS_MODINFO_H__

#include <misc/string.h>
#include <list>

namespace TA3D
{
    class ModInfo
    {
    public:
        typedef std::list<ModInfo> List;
    public:
        /*!
        ** \brief Constructor
        ** \param ID mod ID
        ** \param name mod name
        ** \param version mod version
        ** \param author mod author(s)
        ** \param comment a few things about the mod
        ** \param url where to download the mod
        */
        ModInfo(const int ID = -1, const String &name = String(), const String &version = String(), const String &author = String(), const String &comment = String(), const String &url = String());

        /*!
        ** \brief Constructor
        ** \param info the mod info line returned by netserver, something like MOD "ID" "version" "name" "url" "author" "comment"
        */
        ModInfo(const String &info);

        /*!
        ** \brief Parse the MOD line returned by netserver
        ** \param info the mod info line returned by netserver, something like MOD "ID" "version" "name" "url" "author" "comment"
        */
        void parse(const String &info);

        /*!
        ** \brief Write mod information to the associated info.mod file
        */
        void write();

        /*!
        ** \brief Load mod information from the given info.mod file
        ** \param modName the name of the mod to read
        */
        void read(const String &modName);

		int getID() const			{	return ID;  }
		String getName() const		{	return name;  }
		String getVersion() const	{	return version;  }
		String getAuthor() const	{	return author;  }
		String getComment() const	{	return comment;  }
		String getUrl() const		{	return url;  }
		bool isInstalled() const	{	return installed;	}
		bool isUpdateAvailable() const {	return availableUpdate;	}
		void setUpdateAvailable(bool b)	{	availableUpdate = b;	}
		void setInstalled(bool b);
		void uninstall();

    private:
        int     ID;
        String  name;
        String  version;
        String  author;
        String  comment;
        String  url;
		bool	installed;
		bool	availableUpdate;
    };
}   // namespace TA3D

#endif
