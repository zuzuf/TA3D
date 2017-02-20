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
        ModInfo(const int ID = -1, const QString &name = QString(), const QString &version = QString(), const QString &author = QString(), const QString &comment = QString(), const QString &url = QString());

        /*!
        ** \brief Constructor
        ** \param info the mod info line returned by netserver, something like MOD "ID" "version" "name" "url" "author" "comment"
        */
        ModInfo(const QString &info);

        /*!
        ** \brief Parse the MOD line returned by netserver
        ** \param info the mod info line returned by netserver, something like MOD "ID" "version" "name" "url" "author" "comment"
        */
        void parse(const QString &info);

        /*!
        ** \brief Write mod information to the associated info.mod file
        */
        void write();

        /*!
        ** \brief Load mod information from the given info.mod file
        ** \param modName the name of the mod to read
        */
        void read(const QString &modName);

		int getID() const			{	return ID;  }
		QString getName() const		{	return name;  }
		QString getVersion() const	{	return version;  }
		QString getAuthor() const	{	return author;  }
		QString getComment() const	{	return comment;  }
		QString getUrl() const		{	return url;  }
		QString getPathToMod() const;
		bool isInstalled() const	{	return installed;	}
		bool isUpdateAvailable() const {	return availableUpdate;	}
		void setUpdateAvailable(bool b)	{	availableUpdate = b;	}
		void setInstalled(bool b);
		void uninstall();

        static QString cleanStringForPortablePathName(const QString &s);

    private:
        int     ID;
        QString  name;
        QString  version;
        QString  author;
        QString  comment;
        QString  url;
		bool	installed;
		bool	availableUpdate;
    };
}   // namespace TA3D

#endif
