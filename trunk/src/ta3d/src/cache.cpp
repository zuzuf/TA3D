
#include "cache.h"
#include "misc/string.h"
#include "misc/paths.h"
#include "TA3D_NameSpace.h"
#include <yuni/core/io/file/stream.h>

using namespace Yuni::Core::IO::File;

namespace TA3D
{
namespace Cache
{

	void Clear(const bool force)
	{
		bool rebuild_cache = false;
		// Check cache date
		const String cache_date = lp_CONFIG
			? String("build info : ") << __DATE__ << " , " << __TIME__ << "\ncurrent mod : " << lp_CONFIG->last_MOD << '\n'
			: String("build info : ") << __DATE__ << " , " << __TIME__ << "\ncurrent mod : \n";

		if (Paths::Exists(Paths::Caches + "cache_info.txt") && !force)
		{
			Stream cache_info(Paths::Caches + "cache_info.txt", Yuni::Core::IO::OpenMode::read);
			if (cache_info.opened())
			{
				char *buf = new char[cache_date.size() + 1];
				if (buf)
				{
					::memset(buf, 0, cache_date.size() + 1);
					cache_info.read(buf, cache_date.size());
					if (buf == cache_date)
						rebuild_cache = false;
					else
						rebuild_cache = true;
					DELETE_ARRAY(buf);
				}
				cache_info.close();
			}
		}
		else
			rebuild_cache = true;

		if (rebuild_cache)
		{
			String::List file_list;
			Paths::GlobFiles(file_list, Paths::Caches + "*");
			for (String::List::iterator i = file_list.begin(); i != file_list.end(); ++i)
				remove(i->c_str());
			// Update cache date
			Stream cache_info(Paths::Caches + "cache_info.txt", Yuni::Core::IO::OpenMode::write);
			if (cache_info.opened())
			{
				cache_info.write(cache_date.c_str(), cache_date.size());
				cache_info.put(0);
				cache_info.close();
			}
		}
	}


} // namespace Cache
} // namespace TA3D
