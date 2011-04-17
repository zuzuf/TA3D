
#include "string.h"
#include <logs/logs.h>



namespace TA3D
{

	sint32 SearchString(const String& s, const String& stringToSearch, const bool ignoreCase)
	{
		String sz1, sz2;

		if(ignoreCase)
		{
			sz1 = ToUpper(s);
			sz2 = ToUpper(stringToSearch);
		}
		else
		{
			sz1 = s;
			sz2 = stringToSearch;
		}
		String::Size iFind = sz1.find(sz2);

		return ((String::npos == iFind) ? -1 : (sint32)iFind);
	}



	String InttoUTF8(const uint16 c)
	{
		if (c < 0x80)
		{
			String str;
			str << (char)c;
			return str;
		}
		if (c < 0x800)
		{
			String str;
			int b = 0xC0 | (c >> 6);
			str << (char)b;

			b = 0x80 | (c & 0x3F);
			str << (char)b;
			return str;
		}

		String str;
		int b = 0xC0 | (c >> 12);
		str << (char)b;

		b = 0x80 | ((c >> 6) & 0x3F);
		str << (char)b;

		b = 0x80 | (c & 0x3F);
		str << (char)b;
		return str;
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


	String ConvertToUTF8(const String& s)
	{
		if (s.empty())
			return nullptr;
		char* ret = ConvertToUTF8(s.data(), s.size());
		if (ret)
		{
			String s(ret); // TODO Find a way to not use a temporary string
			DELETE_ARRAY(ret);
			return s;
		}
		return nullptr;
	}

    WString::WString(const char* s)
    {
        if (s)
            fromUtf8(s, strlen(s));
        else
            pBuffer[0] = 0;
    }

    WString::WString(const String& s)
    {
		fromUtf8(s.data(), s.size());
    }

    void WString::fromUtf8(const char* str, size_t length)
    {
		size_t len = 0;
        for (size_t i = 0 ; i < length; i++)
        {
            if (((byte)str[i]) < 0x80)
            {
                pBuffer[len++] = ((byte)str[i]);
                continue;
            }
            if (((byte)str[i]) >= 0xC0)
            {
                wchar_t c = ((byte)str[i++]) - 0xC0;
                while(((byte)str[i]) >= 0x80)
                    c = (c << 6) | (((byte)str[i++]) - 0x80);
                --i;
                pBuffer[len++] = c;
                continue;
            }
        }
        pBuffer[len] = 0;
    }

	String::Vector SplitCommand(const String& s)
	{
		String::Vector args;

		String current;
		bool stringMode = false;
		for (unsigned int i = 0 ; i < s.size(); ++i)
		{
			if (!stringMode)
			{
				if (s[i] == ' ' || s[i] == '"')
				{
					if (!current.empty())
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
			current << s[i];
		}
		if (!current.empty())
			args.push_back(current);

		return args;
	}

	String Escape(const String& s)
	{
		String r(s);
		r.replace("\\", "\\\\");
		r.replace("\"", "\\\"");
		return r;
	}
}
