
#include "cache.h"
#include "misc/string.h"
#include "misc/paths.h"
#include "TA3D_NameSpace.h"
#include <QFile>

namespace TA3D
{
namespace Cache
{

	void Clear(const bool force)
	{
		bool rebuild_cache = false;
		// Check cache date
        const QString cache_info_data = lp_CONFIG
            ? QString("build info : " __DATE__ " , " __TIME__ "\ncurrent mod : ") + lp_CONFIG->last_MOD + '\n'
            : QString("build info : " __DATE__ " , " __TIME__ "\ncurrent mod : \nTexture Quality : %1").arg(lp_CONFIG->unitTextureQuality);

        if (Paths::Exists(Paths::Caches + "cache_info.txt") && !force)
		{
            QFile cache_info(Paths::Caches + "cache_info.txt");
            cache_info.open(QIODevice::ReadOnly);
            if (cache_info.isOpen())
			{
				char *buf = new char[cache_info_data.size() + 1];
				if (buf)
				{
					::memset(buf, 0, cache_info_data.size() + 1);
					cache_info.read(buf, cache_info_data.size());
					if (buf == cache_info_data)
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

		if (lp_CONFIG->developerMode)		// Developer mode forces cache refresh
			rebuild_cache = true;

		if (rebuild_cache)
		{
			QStringList file_list;
            Paths::GlobFiles(file_list, Paths::Caches + "*");
            for (const QString &i : file_list)
                QFile(i).remove();
			// Update cache date
            QFile cache_info(Paths::Caches + "cache_info.txt");
            cache_info.open(QIODevice::WriteOnly);
            if (cache_info.isOpen())
			{
                cache_info.write(cache_info_data.toStdString().c_str(), cache_info_data.size());
                cache_info.putChar(0);
				cache_info.close();
			}
		}
	}


} // namespace Cache
} // namespace TA3D
