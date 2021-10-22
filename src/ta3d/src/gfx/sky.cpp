#include <TA3D_NameSpace.h>
#include "sky.h"
#include <vfs/vfs.h>
#include <misc/tdf.h>

#ifndef M_PI
#define M_PI    3.141592653589793238462643
#endif

namespace TA3D
{

	using namespace TA3D::UTILS;
	using namespace TA3D::VARS;

	Sky::Sky()
	{
		init();
	}

	Sky::~Sky()
	{
		destroy();
	}

	void Sky::init()
	{
        skyInfo = nullptr;
		for(int i = 0 ; i < 6 ; ++i)
            tex[i] = nullptr;
		s = 0;
		w = 1.0f;
	}

	void Sky::destroy()
	{
        skyInfo = nullptr;
		for(int i = 0 ; i < 6 ; ++i)
            tex[i] = nullptr;
		init();
	}

    inline uint32 getpixelBL(const QImage &bmp, float u, float v)
	{
        const float tu = u * float(bmp.width());
        const float tv = v * float(bmp.height());
		const int x = int(tu);
		const int y = int(tv);
		const int dx = int((tu - float(x)) * 65536.0f);
		const int dy = int((tv - float(y)) * 65536.0f);

		uint32 c0, c1, c2, c3;
		sint32 r0, r1, r2, r3;
		sint32 g0, g1, g2, g3;
		sint32 b0, b1, b2, b3;
		sint32 a0, a1, a2, a3;
        const int x0 = x % bmp.width();
        const int x1 = (x + 1) % bmp.width();
        const int y0 = y % bmp.height();
        const int y1 = (y + 1) % bmp.height();
        c0 = SurfaceInt(bmp, x0, y0);
        c1 = SurfaceInt(bmp, x1, y0);
        c2 = SurfaceInt(bmp, x0, y1);
        c3 = SurfaceInt(bmp, x1, y1);

		r0 = getr(c0);	g0 = getg(c0);	b0 = getb(c0);	a0 = geta(c0);
		r1 = getr(c1);	g1 = getg(c1);	b1 = getb(c1);	a1 = geta(c1);
		r2 = getr(c2);	g2 = getg(c2);	b2 = getb(c2);	a2 = geta(c2);
		r3 = getr(c3);	g3 = getg(c3);	b3 = getb(c3);	a3 = geta(c3);

		r0 = r0 + ((r1 - r0) * dx >> 16);
		r2 = r2 + ((r3 - r2) * dx >> 16);
		r0 = r0 + ((r2 - r0) * dy >> 16);

		g0 = g0 + ((g1 - g0) * dx >> 16);
		g2 = g2 + ((g3 - g2) * dx >> 16);
		g0 = g0 + ((g2 - g0) * dy >> 16);

		b0 = b0 + ((b1 - b0) * dx >> 16);
		b2 = b2 + ((b3 - b2) * dx >> 16);
		b0 = b0 + ((b2 - b0) * dy >> 16);

		a0 = a0 + ((a1 - a0) * dx >> 16);
		a2 = a2 + ((a3 - a2) * dx >> 16);
		a0 = a0 + ((a2 - a0) * dy >> 16);
		if (r0 == 0 && g0 == 0 && b0 == 0)
			return makeacol32(1,1,1,a0);
		return makeacol32(r0, g0, b0, a0);
	}

	void Sky::build(int d, float size)
	{
		s = d;
		w = size;

		// Compute the cube map
        QImage stex = gfx->load_image(skyInfo->texture_name);
        if (!stex.isNull())
		{
            convert_format(stex);
			const int skyRes  = std::min<int>(1024, lp_CONFIG->getMaxTextureSizeAllowed());
            QImage img[6];
			for(int i = 0 ; i < 6 ; ++i)
				img[i] = gfx->create_surface(skyRes, skyRes);
			const uint32 fcol = makeacol32(int(skyInfo->FogColor[0] * 255.0f), int(skyInfo->FogColor[1] * 255.0f), int(skyInfo->FogColor[2] * 255.0f), int(skyInfo->FogColor[3] * 255.0f));
			const float coef = 1.0f / float(skyRes - 1);
			float atanfx[8192];
			float invsqrtfx[8192];
			for(int x = 0 ; x < skyRes ; ++x)
			{
				const float fx = 2.0f * (float(x) * coef - 0.5f);
				atanfx[x] = std::atan(fx) / (2.0f * float(M_PI));
				invsqrtfx[x] = 1.0f / sqrtf(1.0f + fx * fx);
			}
            parallel_for<int>(0, skyRes, [&](const int y)
			{
				const float fy = 2.0f * (float(y) * coef - 0.5f);
				for(int x = 0 ; x < skyRes ; ++x)
				{
					const float fx = 2.0f * (float(x) * coef - 0.5f);
					float alpha = atanfx[x];
					float beta = std::atan(fy * invsqrtfx[x]);
					beta = beta / float(M_PI) + 0.5f;
					if (beta <= 0.5f || skyInfo->full_sphere)
					{
						if (!skyInfo->full_sphere)
							beta *= 2.0f;
						beta = Math::Clamp(beta, 0.0f, 1.0f);
						SurfaceInt(img[0], x, y) = getpixelBL(stex, alpha + 1.0f, beta);
						SurfaceInt(img[1], x, y) = getpixelBL(stex, alpha + 1.25f, beta);
						SurfaceInt(img[2], x, y) = getpixelBL(stex, alpha + 1.5f, beta);
						SurfaceInt(img[3], x, y) = getpixelBL(stex, alpha + 1.75f, beta);
					}
					else
					{
						SurfaceInt(img[0], x, y) = fcol;
						SurfaceInt(img[1], x, y) = fcol;
						SurfaceInt(img[2], x, y) = fcol;
						SurfaceInt(img[3], x, y) = fcol;
					}

					alpha = std::atan2(fy, fx) / (2.0f * float(M_PI));
					beta = std::atan(sqrtf(fy * fy + fx * fx)) / float(M_PI);
					if (!skyInfo->full_sphere)
						beta *= 2.0f;

					if (!skyInfo->full_sphere)
						SurfaceInt(img[4], x, y) = fcol;
					else
						SurfaceInt(img[4], x, y) = getpixelBL(stex, alpha + 1.75f, Math::Clamp(1.0f - beta, 0.0f, 1.0f));
					SurfaceInt(img[5], x, y) = getpixelBL(stex, alpha + 1.25f, Math::Clamp(beta, 0.0f, 1.0f));
				}
            });

			gfx->set_texture_format(gfx->defaultTextureFormat_RGB_compressed());
			for(int i = 0 ; i < 6 ; ++i)
				tex[i] = gfx->make_texture(img[i], FILTER_TRILINEAR, true);
		}
	}


	void Sky::draw()
	{
        tex[3]->bind();
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);	glVertex3f(-w, w, w);
			glTexCoord2f(1.0f, 0.0f);	glVertex3f(w, w, w);
			glTexCoord2f(1.0f, 1.0f);	glVertex3f(w, -w, w);
			glTexCoord2f(0.0f, 1.0f);	glVertex3f(-w, -w, w);
		glEnd();
        CHECK_GL();

        tex[2]->bind();
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);	glVertex3f(-w, w, -w);
			glTexCoord2f(1.0f, 0.0f);	glVertex3f(-w, w, w);
			glTexCoord2f(1.0f, 1.0f);	glVertex3f(-w, -w, w);
			glTexCoord2f(0.0f, 1.0f);	glVertex3f(-w, -w, -w);
		glEnd();
        CHECK_GL();

        tex[1]->bind();
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);	glVertex3f(w, w, -w);
			glTexCoord2f(1.0f, 0.0f);	glVertex3f(-w, w, -w);
			glTexCoord2f(1.0f, 1.0f);	glVertex3f(-w, -w, -w);
			glTexCoord2f(0.0f, 1.0f);	glVertex3f(w, -w, -w);
		glEnd();
        CHECK_GL();

        tex[0]->bind();
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);	glVertex3f(w, w, w);
			glTexCoord2f(1.0f, 0.0f);	glVertex3f(w, w, -w);
			glTexCoord2f(1.0f, 1.0f);	glVertex3f(w, -w, -w);
			glTexCoord2f(0.0f, 1.0f);	glVertex3f(w, -w, w);
		glEnd();
        CHECK_GL();

        tex[4]->bind();
		glBegin(GL_QUADS);
			glTexCoord2f(1.0f, 0.0f);	glVertex3f(-w, -w, w);
			glTexCoord2f(1.0f, 1.0f);	glVertex3f(w, -w, w);
			glTexCoord2f(0.0f, 1.0f);	glVertex3f(w, -w, -w);
			glTexCoord2f(0.0f, 0.0f);	glVertex3f(-w, -w, -w);
		glEnd();
        CHECK_GL();

        tex[5]->bind();
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);	glVertex3f(w, w, w);
			glTexCoord2f(0.0f, 1.0f);	glVertex3f(-w, w, w);
			glTexCoord2f(1.0f, 1.0f);	glVertex3f(-w, w, -w);
			glTexCoord2f(1.0f, 0.0f);	glVertex3f(w, w, -w);
		glEnd();
        CHECK_GL();
    }

	void Sky::SkyData::load_tdf(const QString& filename)
	{
		TDFParser parser;
		if (!parser.loadFromFile(filename))
			LOG_ERROR("Impossible to load the sky data from `" << filename << "`");
		def = parser.pullAsBool("sky.default", false);
		full_sphere = parser.pullAsBool("sky.full sphere");
		rotation_speed = parser.pullAsFloat("sky.rotation speed");
		rotation_offset = parser.pullAsFloat("sky.rotation offset");
		texture_name = parser.pullAsString("sky.texture name");
        planet = parser.pullAsString("sky.planet").split(',');
		FogColor[0] = parser.pullAsFloat("sky.fog R");
		FogColor[1] = parser.pullAsFloat("sky.fog G");
		FogColor[2] = parser.pullAsFloat("sky.fog B");
		FogColor[3] = parser.pullAsFloat("sky.fog A");
        MapName = parser.pullAsString("sky.map").split(',');
	}


	void Sky::choose_a_sky(const QString& mapname, const QString& planet)
	{
        skyInfo = nullptr;

        std::vector<SkyData::Ptr> sky_list;
		sky_list.clear();

		QStringList file_list;
        VFS::Instance()->getFilelist("sky/*.tdf", file_list);
		uint32	nb_sky = 0;

        for (const QString &it : file_list)
		{
            LOG_DEBUG("loading sky : " << it);
            SkyData::Ptr sky_data = new SkyData;
            sky_data->load_tdf(it);

			bool keep = false;
            for (const QString &i : sky_data->MapName)
			{
                if (i == mapname)
				{
					keep = true;
					break;
				}
			}
			if (!keep)
			{
				QString sky;
				QString p;
                for (const QString &i : sky_data->planet)
				{
                    sky = i;
					p = planet;
					if (sky.toLower() == p.toLower())
					{
						keep = true;
						break;
					}
				}
			}
			if (keep)
			{
				sky_list.push_back(sky_data);
				++nb_sky;
			}
		}

		if (nb_sky == 0)    // Look for a default sky
		{
			LOG_DEBUG(LOG_PREFIX_GFX << "no sky associated with this map('" << mapname << "') or this planet('" << planet << "') found, looking for default skies");
            for (const QString &it : file_list)
			{
                SkyData::Ptr sky_data = new SkyData;
                sky_data->load_tdf(it);

				bool keep = sky_data->def;
				if (keep)
				{
					sky_list.push_back(sky_data);
					++nb_sky;
				}
			}
		}

        SkyData::Ptr selected_sky;

		if (nb_sky > 0)
		{
			int select = TA3D_RAND() % nb_sky;
            for (const SkyData::Ptr &it : sky_list)
			{
				if (select == 0)
				{
                    selected_sky = it;
					break;
				}
                --select;
			}
		}

		sky_list.clear();

		skyInfo = selected_sky;
		// Temporary
		build(10, 400);
	}



	Sky::SkyData::SkyData()
	{
		rotation_offset = 0.0f;
		full_sphere = false;
		rotation_speed = 0.0f;
		texture_name.clear();
		planet.clear();
		FogColor[0] = 0.8f;
		FogColor[1] = 0.8f;
		FogColor[2] = 0.8f;
		FogColor[3] = 1.0f;
		def = false;
		MapName.clear();
	}

	Sky::SkyData::~SkyData()
	{
		texture_name.clear();
		planet.clear();
		MapName.clear();
	}
}
