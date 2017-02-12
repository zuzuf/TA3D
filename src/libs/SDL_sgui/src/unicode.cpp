
#include <SDL/sgui/sdl-headers.h>
#include <SDL/sgui/unicode.h>

using namespace std;

namespace Gui
{

	std::string toUtf8(const wstring &wstr)
	{
		std::string str;
		str.reserve(wstr.size());

		for(wstring::const_iterator i = wstr.begin() ; i != wstr.end() ; ++i)
		{
			if (*i <= 0x7F)
				str += *i;
			else if (*i <= 0x7FF)
			{
				str += 0xC0 | (*i >> 6);
				str += 0x80 | (*i & 0x3F);
			}
			else
			{
				str += 0xE0 | (*i >> 12);
				str += 0x80 | ((*i >> 6) & 0x3F);
				str += 0x80 | (*i & 0x3F);
			}
		}

		return str;
	}

	std::wstring fromUtf8(const string &str)
	{
		std::wstring wstr;
		wstr.reserve(str.size());

		for(string::const_iterator i = str.begin() ; i != str.end() ; ++i)
		{
			if (!(*i & 0x80))
				wstr += *i;
			else if ((*i & 0xE0) == 0xC0)
			{
				wchar_t w;
				w = (wchar_t(*i) & 0x3F) << 6;	++i;
				w |= (wchar_t(*i) & 0x3F);
				wstr += w;
			}
			else
			{
				wchar_t w;
				w = (wchar_t(*i) & 0xF) << 12;	++i;
				w |= (wchar_t(*i) & 0x3F) << 6;	++i;
				w |= (wchar_t(*i) & 0x3F);
				wstr += w;
			}
		}

		return wstr;
	}

	ostream &operator<<(ostream &out, const ustring &str)
	{
		return (out << toUtf8(str));
	}
}
