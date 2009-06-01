#ifndef __TA3D_MISC_MODINFO_H__
# define __TA3D_MISC_MODINFO_H__

#include "string.h"
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

        inline int getID()          {   return ID;  }
        inline String getName()     {   return name;  }
        inline String getVersion()  {   return version;  }
        inline String getAuthor()   {   return author;  }
        inline String getComment()  {   return comment;  }
        inline String getUrl()      {   return url;  }

    private:
        int     ID;
        String  name;
        String  version;
        String  author;
        String  comment;
        String  url;
    };
}   // namespace TA3D

#endif
