#ifndef __YUNI_CORE_URI_URI_HXX__
# define __YUNI_CORE_URI_URI_HXX__


namespace Yuni
{


	inline Uri::Uri()
	{}


	inline void Uri::clear()
	{
		pInfos.clear();
	}


	inline const String& Uri::scheme() const
	{
		return pInfos.scheme;
	}

	inline const String& Uri::server() const
	{
		return pInfos.server;
	}

	inline int Uri::port() const
	{
		return pInfos.port;
	}

	inline const String& Uri::user() const
	{
		return pInfos.user;
	}


	inline const String& Uri::password() const
	{
		return pInfos.password;
	}

	inline void Uri::password(const String& s)
	{
		pInfos.password = s;
		pInfos.isValid = true;
	}

	inline const String& Uri::path() const
	{
		return pInfos.path;
	}

	inline const String& Uri::query() const
	{
		return pInfos.query;
	}

	inline const String& Uri::fragment() const
	{
		return pInfos.fragment;
	}

	inline void Uri::fragment(const String& s)
	{
		pInfos.fragment = s;
		pInfos.isValid = true;
	}



	inline Net::Protocol::Type Uri::protocol() const
	{
		return pInfos.isValid
			? Net::Protocol::SchemeToType(pInfos.scheme)
			: Net::Protocol::unknown;
	}


	inline bool Uri::schemeIsFile() const
	{
		return (pInfos.scheme.empty() && !pInfos.path.empty()) || "file" == pInfos.scheme;
	}

	inline bool Uri::schemeIsHTTP() const
	{
		return !pInfos.scheme.empty() && ("http" == pInfos.scheme || "https" == pInfos.scheme);
	}

	inline bool Uri::schemeIsFTP() const
	{
		return !pInfos.scheme.empty() && "ftp" == pInfos.scheme;
	}

	inline bool Uri::schemeIsSSH() const
	{
		return !pInfos.scheme.empty() && "ssh" == pInfos.scheme;
	}

	inline bool Uri::schemeIsLDAP() const
	{
		return !pInfos.scheme.empty() && "ldap" == pInfos.scheme;
	}



	inline bool Uri::isValid() const
	{
		return pInfos.isValid;
	}





	inline Uri& Uri::operator = (const Uri& rhs)
	{
		pInfos.assign(rhs.pInfos);
		return *this;
	}

	inline Uri& Uri::operator = (const String& rhs)
	{
		extractURIFromString(rhs);
		return *this;
	}

	inline bool Uri::operator == (const Uri& rhs) const
	{
		return pInfos.isEqualsTo(rhs.pInfos);
	}

	inline bool Uri::operator != (const Uri& rhs) const
	{
		return !((*this) == rhs);
	}

	inline String Uri::operator() () const
	{
		return pInfos.toString();
	}


	inline void Uri::scheme(const Net::Protocol::Type& type)
	{
		pInfos.scheme = Net::Protocol::ToScheme(type);
		pInfos.isValid = true;
	}




} // namespace Yuni

#endif // __YUNI_CORE_URI_URI_H__
