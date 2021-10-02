#include "string.h"
#include <logs/logs.h>


namespace TA3D
{

    sint32 SearchString(const QString& s, const QString& stringToSearch, const bool ignoreCase)
	{
        const QString::size_type iFind = s.indexOf(stringToSearch, 0, ignoreCase ? Qt::CaseInsensitive : Qt::CaseSensitive);

        return ((-1 == iFind) ? -1 : (sint32)iFind);
	}

    int ASCIItoUTF8(const byte c, byte *out)
    {
        if (c < 0x80)
        {
            *out = c;
            return 1;
        }
        else if(c < 0xC0)
        {
            out[0] = 0xC2;
            out[1] = c;
            return 2;
        }
        out[0] = 0xC3;
        out[1] = static_cast<byte>(c - 0x40);
        return 2;
    }

    char* ConvertToUTF8(const char* s)
    {
        if (NULL != s && *s != '\0')
            return ConvertToUTF8(s, strlen(s));
        char* ret = new char[1];
        LOG_ASSERT(NULL != ret);
        *ret = '\0';
        return ret;
    }

    char* ConvertToUTF8(const char* s, const size_t len)
    {
        uint32 nws;
        return ConvertToUTF8(s, len, nws);
    }

    char* ConvertToUTF8(const char* s, size_t size, uint32& newSize)
    {
        if (NULL == s || '\0' == *s)
        {
            char* ret = new char[1];
            LOG_ASSERT(NULL != ret);
            *ret = '\0';
            return ret;
        }
        byte tmp[4];
        newSize = 1;
        size_t n = size;
        for(byte *p = (byte*)s ; *p && n ; ++p, --n)
            newSize += ASCIItoUTF8(*p, tmp);

        char* ret = new char[newSize];
        LOG_ASSERT(NULL != ret);

        byte *q = (byte*)ret;
        n = size;
        for(byte *p = (byte*)s ; *p && n ; ++p, --n)
            q += ASCIItoUTF8(*p, q);
        *q = '\0'; // A bit paranoid
        return ret;
    }

    QStringList SplitCommand(const QString& s)
	{
        QStringList args;

        QString current;
		bool stringMode = false;
		for (unsigned int i = 0 ; i < s.size(); ++i)
		{
			if (!stringMode)
			{
				if (s[i] == ' ' || s[i] == '"')
				{
                    if (!current.isEmpty())
					{
						args.push_back(current);
						current.clear();
					}
					if (s[i] == '"')
						stringMode = true;
					continue;
				}
			}
			else
			{
				if (s[i] == '\\' && i + 1 < s.size() && (s[i+1] == '"' || s[i+1] == '\\'))
					++i;
				else if (s[i] == '"')
				{
					stringMode = false;
					args.push_back(current);
					current.clear();
					continue;
				}
			}
            current.push_back(s[i]);
		}
        if (!current.isEmpty())
			args.push_back(current);

		return args;
	}

    QString Escape(const QString& s)
	{
        QString r(s);
        r.replace("\\", "\\\\");
        r.replace("\"", "\\\"");
		return r;
	}
}
