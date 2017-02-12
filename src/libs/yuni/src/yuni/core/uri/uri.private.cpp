
#include "uri.private.h"


namespace Yuni
{
namespace Private
{
namespace Uri
{


	Informations::Informations()
		:port(0), isValid(false)
	{}


	Informations::Informations(const Informations& rhs)
		:scheme(rhs.scheme), user(rhs.user), password(rhs.password), server(rhs.server),
		port(rhs.port), path(rhs.path), query(rhs.query), fragment(rhs.fragment),
		isValid(rhs.isValid)
	{}


	Informations::~Informations()
	{}



	void Informations::clear()
	{
		scheme.clear();
		server.clear();
		user.clear();
		password.clear();
		port = 0;
		path.clear();
		query.clear();
		fragment.clear();
		isValid = false;
	}


	void Informations::assign(const Informations& rhs)
	{
		scheme   = rhs.scheme;
		server   = rhs.server;
		user     = rhs.user;
		port     = rhs.port;
		path     = rhs.path;
		query    = rhs.query;
		fragment = rhs.fragment;
		isValid  = rhs.isValid;
	}


	namespace
	{
		template<class U>
		void WriteStructInformationsToStream(const Informations& infos, U& s)
		{
			if (infos.isValid)
			{
				if (!infos.scheme.empty())
					s << infos.scheme << ":";
				if (!infos.server.empty())
				{
					if (!infos.scheme.empty())
						s << "//";
					if (!infos.user.empty())
					{
						s << infos.user;
						if (!infos.password.empty())
							s << ":" << infos.password;
						s << "@";
					}
					s << infos.server;
					if (infos.port > 0)
						s << ":" << infos.port;
				}
				else
				{
					if (!infos.scheme.empty() && "file" == infos.scheme)
						s << "//";
				}
				s << infos.path;
				if (!infos.query.empty())
					s << "?" << infos.query;
				if (!infos.fragment.empty())
					s << "#" << infos.fragment;
			}
		}

	} // Anonymous namespace


	String Informations::toString() const
	{
		if (isValid)
		{
			String s;
			WriteStructInformationsToStream(*this, s);
			return s;
		}
		return String();
	}


	void Informations::print(std::ostream& out) const
	{
		if (isValid)
			WriteStructInformationsToStream(*this, out);
	}



	bool Informations::isEqualsTo(const Informations& rhs) const
	{
		return isValid && rhs.isValid && scheme == rhs.scheme
			&& path == rhs.path && server == rhs.server && port == rhs.port
			&& query == rhs.query && fragment == rhs.fragment
			&& user == rhs.user && password == rhs.password;
	}




} // namespace Uri
} // namespace Private
} // namespace Yuni

