#include <TA3D_NameSpace.h>
#include "sky.h"
#include <vfs/vfs.h>
#include <misc/tdf.h>

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
		skyInfo = NULL;
		for(int i = 0 ; i < 6 ; ++i)
			tex[i] = 0;
		dlist = 0;
		s = 0;
		w = 1.0f;
	}

	void Sky::destroy()
	{
		if (skyInfo)
			delete skyInfo;
		for(int i = 0 ; i < 6 ; ++i)
			gfx->destroy_texture(tex[i]);
		if (dlist)
			glDeleteLists(dlist, 1);
		init();
	}

	inline uint32 getpixelBL(SDL_Surface *bmp, float u, float v)
	{
		const float tu = u * bmp->w;
		const float tv = v * bmp->h;
		const int x = int(tu);
		const int y = int(tv);
		const int dx = int((tu - x) * 65536.0f);
		const int dy = int((tv - y) * 65536.0f);

		uint32 c[4];
		c[0] = getpixel(bmp, x % bmp->w, y % bmp->h);
		c[1] = getpixel(bmp, (x + 1) % bmp->w, y % bmp->h);
		c[2] = getpixel(bmp, x % bmp->w, (y + 1) % bmp->h);
		c[3] = getpixel(bmp, (x + 1) % bmp->w, (y + 1) % bmp->h);

		int r[4], g[4], b[4];
		for(int i = 0 ; i < 4 ; ++i)
		{
			r[i] = getr(c[i]);
			g[i] = getg(c[i]);
			b[i] = getb(c[i]);
		}
		r[0] = r[0] + ((r[1] - r[0]) * dx >> 16);
		r[2] = r[2] + ((r[3] - r[2]) * dx >> 16);
		r[0] = r[0] + ((r[2] - r[0]) * dy >> 16);

		g[0] = g[0] + ((g[1] - g[0]) * dx >> 16);
		g[2] = g[2] + ((g[3] - g[2]) * dx >> 16);
		g[0] = g[0] + ((g[2] - g[0]) * dy >> 16);

		b[0] = b[0] + ((b[1] - b[0]) * dx >> 16);
		b[2] = b[2] + ((b[3] - b[2]) * dx >> 16);
		b[0] = b[0] + ((b[2] - b[0]) * dy >> 16);

		return makecol(r[0], g[0], b[0]);
	}

	void Sky::build(int d,float size)
	{
		s = d;
		w = size;

		// Compute the cube map
		SDL_Surface *stex = gfx->load_image(skyInfo->texture_name);
		if (stex)
		{
			stex = convert_format(stex);
			const int skyRes = 1024;
			SDL_Surface *img[6];
			for(int i = 0 ; i < 6 ; ++i)
				img[i] = gfx->create_surface(skyRes, skyRes);
			const uint32 fcol = makeacol32(int(skyInfo->FogColor[0] * 255.0f), int(skyInfo->FogColor[1] * 255.0f), int(skyInfo->FogColor[2] * 255.0f), int(skyInfo->FogColor[3] * 255.0f));
			const float coef = 1.0f / (skyRes - 1);
			float atanfx[skyRes];
			float invsqrtfx[skyRes];
			for(int x = 0 ; x < skyRes ; ++x)
			{
				const float fx = 2.0f * (x * coef - 0.5f);
				atanfx[x] = atan(fx) / (2.0f * M_PI);
				invsqrtfx[x] = 1.0f / sqrtf(1.0f + fx * fx);
			}
			for(int y = 0 ; y < skyRes ; ++y)
			{
				const float fy = 2.0f * (y * coef - 0.5f);
				for(int x = 0 ; x < skyRes ; ++x)
				{
					const float fx = 2.0f * (x * coef - 0.5f);
					float alpha = atanfx[x];
					float beta = atan(fy * invsqrtfx[x]);
					beta = beta / M_PI + 0.5f;
					if (beta <= 0.5f || skyInfo->full_sphere)
					{
						if (!skyInfo->full_sphere)
							beta *= 2.0f;
						beta = Math::Clamp(beta, 0.0f, 1.0f);
						putpixel(img[0], x, y, getpixelBL(stex, alpha + 1.0f, beta));
						putpixel(img[1], x, y, getpixelBL(stex, alpha + 1.25f, beta));
						putpixel(img[2], x, y, getpixelBL(stex, alpha + 1.5f, beta));
						putpixel(img[3], x, y, getpixelBL(stex, alpha + 1.75f, beta));
					}
					else
					{
						putpixel(img[0], x, y, fcol);
						putpixel(img[1], x, y, fcol);
						putpixel(img[2], x, y, fcol);
						putpixel(img[3], x, y, fcol);
					}

					alpha = atan2(fy, fx) / (2.0f * M_PI);
					beta = atan(sqrtf(fy * fy + fx * fx)) / M_PI;
					if (!skyInfo->full_sphere)
						beta *= 2.0f;

					if (!skyInfo->full_sphere)
						putpixel(img[4], x, y, fcol);
					else
						putpixel(img[4], x, y, getpixelBL(stex, alpha + 1.75f, Math::Clamp(1.0f - beta, 0.0f, 1.0f)));
					putpixel(img[5], x, y, getpixelBL(stex, alpha + 1.25f, Math::Clamp(beta, 0.0f, 1.0f)));
				}
			}

			gfx->set_texture_format(gfx->defaultTextureFormat_RGB());
			for(int i = 0 ; i < 6 ; ++i)
			{
				tex[i] = gfx->make_texture(img[i], FILTER_TRILINEAR, true);
				SDL_FreeSurface(img[i]);
			}
			SDL_FreeSurface(stex);
		}
	}


	void Sky::draw()
	{
		if (dlist)
		{
			glCallList(dlist);
			return;
		}
		dlist = glGenLists(1);
		glNewList(dlist, GL_COMPILE_AND_EXECUTE);

		glBindTexture(GL_TEXTURE_2D, tex[3]);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);	glVertex3f(-w, w, w);
			glTexCoord2f(1.0f, 0.0f);	glVertex3f(w, w, w);
			glTexCoord2f(1.0f, 1.0f);	glVertex3f(w, -w, w);
			glTexCoord2f(0.0f, 1.0f);	glVertex3f(-w, -w, w);
		glEnd();

		glBindTexture(GL_TEXTURE_2D, tex[2]);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);	glVertex3f(-w, w, -w);
			glTexCoord2f(1.0f, 0.0f);	glVertex3f(-w, w, w);
			glTexCoord2f(1.0f, 1.0f);	glVertex3f(-w, -w, w);
			glTexCoord2f(0.0f, 1.0f);	glVertex3f(-w, -w, -w);
		glEnd();

		glBindTexture(GL_TEXTURE_2D, tex[1]);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);	glVertex3f(w, w, -w);
			glTexCoord2f(1.0f, 0.0f);	glVertex3f(-w, w, -w);
			glTexCoord2f(1.0f, 1.0f);	glVertex3f(-w, -w, -w);
			glTexCoord2f(0.0f, 1.0f);	glVertex3f(w, -w, -w);
		glEnd();

		glBindTexture(GL_TEXTURE_2D, tex[0]);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);	glVertex3f(w, w, w);
			glTexCoord2f(1.0f, 0.0f);	glVertex3f(w, w, -w);
			glTexCoord2f(1.0f, 1.0f);	glVertex3f(w, -w, -w);
			glTexCoord2f(0.0f, 1.0f);	glVertex3f(w, -w, w);
		glEnd();

		glBindTexture(GL_TEXTURE_2D, tex[4]);
		glBegin(GL_QUADS);
			glTexCoord2f(1.0f, 0.0f);	glVertex3f(-w, -w, w);
			glTexCoord2f(1.0f, 1.0f);	glVertex3f(w, -w, w);
			glTexCoord2f(0.0f, 1.0f);	glVertex3f(w, -w, -w);
			glTexCoord2f(0.0f, 0.0f);	glVertex3f(-w, -w, -w);
		glEnd();

		glBindTexture(GL_TEXTURE_2D, tex[5]);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);	glVertex3f(w, w, w);
			glTexCoord2f(0.0f, 1.0f);	glVertex3f(-w, w, w);
			glTexCoord2f(1.0f, 1.0f);	glVertex3f(-w, w, -w);
			glTexCoord2f(1.0f, 0.0f);	glVertex3f(w, w, -w);
		glEnd();

		glEndList();
	}

	void Sky::SkyData::load_tdf(const String& filename)
	{
		TDFParser parser;
		if (!parser.loadFromFile(filename))
			LOG_ERROR("Impossible to load the sky data from `" << filename << "`");
		def = parser.pullAsBool("sky.default", false);
		spherical = parser.pullAsBool("sky.spherical");
		full_sphere = parser.pullAsBool("sky.full sphere");
		rotation_speed = parser.pullAsFloat("sky.rotation speed");
		rotation_offset = parser.pullAsFloat("sky.rotation offset");
		texture_name = parser.pullAsString("sky.texture name");
		parser.pullAsString("sky.planet").explode(planet, ',');
		FogColor[0] = parser.pullAsFloat("sky.fog R");
		FogColor[1] = parser.pullAsFloat("sky.fog G");
		FogColor[2] = parser.pullAsFloat("sky.fog B");
		FogColor[3] = parser.pullAsFloat("sky.fog A");
		parser.pullAsString("sky.map").explode(MapName, ',');
	}


	void Sky::choose_a_sky(const String& mapname, const String& planet)
	{
		if (skyInfo)
			delete skyInfo;
		skyInfo = NULL;

		std::vector<SkyData*> sky_list;
		sky_list.clear();

		String::Vector file_list;
		VFS::Instance()->getFilelist("sky\\*.tdf", file_list);
		uint32	nb_sky = 0;

		for (String::Vector::const_iterator it = file_list.begin(); it != file_list.end(); ++it)
		{
			LOG_DEBUG("loading sky : " << *it);
			SkyData *sky_data = new SkyData;
			sky_data->load_tdf(*it);

			bool keep = false;
			for (String::Vector::const_iterator i = sky_data->MapName.begin(); i != sky_data->MapName.end(); ++i)
			{
				if (*i == mapname)
				{
					keep = true;
					break;
				}
			}
			if (!keep)
			{
				String sky;
				String p;
				for (String::Vector::const_iterator i = sky_data->planet.begin(); i != sky_data->planet.end(); ++i)
				{
					sky = *i;
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
			else
				delete sky_data;
		}

		if (nb_sky == 0)    // Look for a default sky
		{
			LOG_DEBUG(LOG_PREFIX_GFX << "no sky associated with this map('" << mapname << "') or this planet('" << planet << "') found, looking for default skies");
			for (String::Vector::const_iterator it = file_list.begin(); it != file_list.end(); ++it)
			{
				SkyData *sky_data = new SkyData;
				sky_data->load_tdf(*it);

				bool keep = sky_data->def;
				if (keep)
				{
					sky_list.push_back(sky_data);
					++nb_sky;
				}
				else
					delete sky_data;
			}
		}

		SkyData *selected_sky = NULL;

		if (nb_sky > 0)
		{
			int select = TA3D_RAND() % nb_sky;
			for (std::vector<SkyData*>::iterator it = sky_list.begin() ; it != sky_list.end(); ++it, --select)
			{
				if (select == 0)
				{
					selected_sky = *it;
					*it = NULL;
					break;
				}
			}
		}

		for (std::vector<SkyData*>::iterator it = sky_list.begin() ; it != sky_list.end(); ++it)
		{
			if (*it != NULL )
				delete *it;
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
		spherical = false;
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