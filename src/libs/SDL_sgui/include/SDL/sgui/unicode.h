#ifndef UNICODE_H
#define UNICODE_H

#include <string>
#include <ostream>

namespace Gui
{
	std::string toUtf8(const std::wstring &wstr);
	inline const std::string &toUtf8(const std::string &str)	{	return str;	}
	std::wstring fromUtf8(const std::string &str);
	inline const std::wstring &fromUtf8(const std::wstring &wstr)	{	return wstr;	}

	class ustring : public std::wstring
	{
	public:
		ustring()	{}
		ustring(const std::string &s) : std::wstring(fromUtf8(s))	{}
		ustring(const char *s) : std::wstring(s ? fromUtf8(s) : L"")	{}
		ustring(const std::wstring &s) : std::wstring(s)	{}
		ustring(const wchar_t *s) : std::wstring(s ? s : L"")	{}

		operator std::string() const {	return Gui::toUtf8(*this);	}

		std::string toUtf8() const {	return Gui::toUtf8(*this);	}
	};

	std::ostream &operator<<(std::ostream &out, const ustring &str);
}

#endif // UNICODE_H
