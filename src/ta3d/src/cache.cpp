
#include "cache.h"
#include "misc/string.h"
#include "misc/paths.h"
#include "TA3D_NameSpace.h"


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
			FILE *cache_info = TA3D_OpenFile(Paths::Caches + "cache_info.txt", "rb");
			if (cache_info)
			{
				char *buf = new char[cache_date.size() + 1];
				if (buf)
				{
					::memset(buf, 0, cache_date.size() + 1);
					::fread(buf, cache_date.size(), 1, cache_info);
					if (buf == cache_date)
						rebuild_cache = false;
					else
						rebuild_cache = true;
					DELETE_ARRAY(buf);
				}
				::fclose(cache_info);
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
			FILE *cache_info = TA3D_OpenFile(Paths::Caches + "cache_info.txt", "wb");
			if (cache_info)
			{
				::fwrite(cache_date.c_str(), cache_date.size(), 1, cache_info);
				::putc(0, cache_info);
				::fclose(cache_info);
			}
		}
	}


} // namespace Cache
} // namespace TA3D
