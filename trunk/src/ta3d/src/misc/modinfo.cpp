#include "../logs/logs.h"
#include "modinfo.h"

namespace TA3D
{
    ModInfo::ModInfo(const int ID, const String &name, const String &version, const String &author, const String &comment, const String &url)
        : ID(ID), name(name), version(version), author(author), comment(comment), url(url)
    {
    }

    ModInfo::ModInfo(const String &info)
    {
        parse(info);
    }

    void ModInfo::parse(const String &info)
    {
        String::Vector params;

        String::Size pos = 0;
        while((pos = info.find_first_of("\"", pos)) != String::npos)
        {
            ++pos;
            String p;
            while(pos < info.size() && info[pos] != '"')
            {
                if (info[pos] == '\\' && pos + 1 < info.size())
                {
                    ++pos;
                    p << info[pos];
                }
                else
                    p << info[pos];
                ++pos;
            }
            ++pos;
            params.push_back(p);
        }

        if (params.size() != 6)     // If there is not the expected number of parameters, then we can't trust this information
        {
            *this = ModInfo();
            return;
        }

        ID = params[0].to<sint32>();
        version = params[1];
        name = params[2];
        url = params[3];
        author = params[4];
        comment = params[5];
    }
}
