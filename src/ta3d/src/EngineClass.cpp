/*  TA3D, a remake of Total Annihilation
	Copyright (C) 2005   Roland BROCHARD

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/

/*----------------------------------------------------------\
  |                      EngineClass.cpp                      |
  |    Contient toutes les classes nécessaires au moteur 3D   |
  | et au moteur physique.                                    |
  |                                                           |
  \----------------------------------------------------------*/

#include "stdafx.h"
#include "misc/matrix.h"
#include "TA3D_NameSpace.h"
#include "ta3dbase.h"
#include "scripts/cob.h"					// Pour la lecture et l'éxecution des scripts
#include "tdf.h"					// Pour la gestion des éléments du jeu
#include "EngineClass.h"
#include "UnitEngine.h"
#include "gfx/fx.h"
#include "misc/math.h"
#include "logs/logs.h"
#include "misc/tdf.h"

#ifndef M_PI
#define M_PI    3.141592653589793238462643
#endif



namespace TA3D
{
	unsigned int player_color_map[TA3D_PLAYERS_HARD_LIMIT] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };


	const float	player_color[TA3D_PLAYERS_HARD_LIMIT * 3] =
	{	0.11f,	0.28f,	0.91f,
		0.83f,	0.17f,	0.0f,
		1.0f,	1.0f,	1.0f,
		0.11f,	0.62f,	0.07f,
		0.03f,	0.12f,	0.48f,
		0.5f,	0.34f,	0.62f,
		1.0f,	1.0f,	0.0f,
		0.0f,	0.0f,	0.0f,
		0.61f,	0.8f,	0.87f,
		0.67f,	0.67f,	0.51f
	};



	MAP* the_map = NULL;


	void SECTOR::init()
	{
		stuff = -1;
		unit_idx = -1;
		flags = 0U;
	}

	void MAP_OTA::init()
	{
		network = false;
		planet.clear();
		glamour.clear();
		missionname.clear();
		missiondescription.clear();
		numplayers.clear();
		map_size.clear();
		for (int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
			startX[i] = startZ[i] = 0;
		tidalstrength = 0;
		solarstrength = 22;
		lavaworld = false;
		killmul = 50;
		minwindspeed = 0;
		maxwindspeed = 0;
		gravity = 9.8f;
		SurfaceMetal = 0;
		MohoMetal = 0;
		waterdamage = 0;
		waterdoesdamage = false;
	}


	void MAP_OTA::destroy()
	{
		glamour.clear();
		planet.clear();
		missionname.clear();
		missiondescription.clear();
		numplayers.clear();
		map_size.clear();
		init();
	}


	void MAP::init()
	{
		/*------------- Experimental: code for new map format -----------------------*/

		/*		macro_bloc = NULL;
				macro_w = 0;
				macro_h = 0;*/

		/*---------------------------------------------------------------------------*/
		view_map.clear();
		sight_map.clear();
		radar_map.clear();
		sonar_map.clear();

        shadow2_shader = new Shader("shaders/map_shadow.frag", "shaders/map_shadow.vert");
        detail_shader = new Shader( "shaders/details.frag", "shaders/details.vert" );
		details_tex = 0;
		color_factor = 1.0f;

		low_nb_idx = 0;
		low_vtx = NULL;
		low_vtx_flat = NULL;
		low_tcoord = NULL;
		low_col = NULL;
		low_index = NULL;
        low_tex = nullptr;

		wind = 0.0f;
		wind_dir = 0.0f;
		wind_vec.reset();
		ota_data.init();
        lava_map = nullptr;
		mini_w = mini_h = 252;
        tex.clear();
		nbbloc = 0;
		bloc = NULL;
        mini = QImage();
		bmap.resize(0,0);
		h_map.resize(0,0);
		ph_map.resize(0,0);
		map_data.resize(0,0);
		sealvl = 0.0f;
        glmini = nullptr;
		lvl = NULL;
		water = true;
		tnt = false;			// Laisse la possibilité de créer un autre format de cartes
		sea_dec = 0.0f;
		view.resize(0,0);
		ox1 = ox2 = oy1 = oy2 = 0;
		int buf_size = 0;
		for (int i = 0; i < 6500; ++i)
		{
			buf_i[i++] = GLushort(0 + buf_size);
			buf_i[i++] = GLushort(1 + buf_size);
			buf_i[i++] = GLushort(3 + buf_size);
			buf_i[i++] = GLushort(4 + buf_size);
			buf_i[i++] = GLushort(6 + buf_size);
			buf_i[i++] = GLushort(7 + buf_size);
			buf_i[i++] = GLushort(7 + buf_size);
			buf_i[i++] = GLushort(8 + buf_size);
			buf_i[i++] = GLushort(4 + buf_size);
			buf_i[i++] = GLushort(5 + buf_size);
			buf_i[i++] = GLushort(1 + buf_size);
			buf_i[i++] = GLushort(2 + buf_size);
			buf_i[i]   = GLushort(2 + buf_size);
			buf_size += 9;
		}
	}



	float MAP::get_unit_h(float x, float y) const
	{
		if (isNaN(x) || isNaN(y))
			return 0.0f;
		x = (x + (float)map_w_d) * 0.125f;		// Convertit les coordonnées
		y = (y + (float)map_h_d) * 0.125f;
		const int lx = bloc_w_db - 1;
		const int ly = bloc_h_db - 1;
		x = Math::Clamp(x, 0.0f, float(bloc_w_db - 2));
		y = Math::Clamp(y, 0.0f, float(bloc_h_db - 2));
		float h[4];
		const int X = Math::Clamp((int)x, 0, lx - 1), Y = Math::Clamp((int)y, 0, ly - 1);
		const float dx = x - (float)X;
		const float dy = y - (float)Y;
		h[0] = h_map(X, Y);
		if (X + 1 < lx)
			h[1] = h_map(X + 1, Y) - h[0];
		else
			h[1] = 0.0f;

		if (Y + 1 < ly)
		{
			h[2] = h_map(X, Y+1);
			if (X + 1 < lx)
				h[3] = h_map(X + 1, Y + 1) - h[2];
			else
				h[3] = 0.0f;
		}
		else
		{
			h[2] = h[0];
			h[3] = h[1];
		}
		h[0] = h[0] + h[1] * dx;
		return h[0] + (h[2] + h[3] * dx - h[0]) * dy;
	}


	float MAP::get_max_h(int x,int y) const
	{
		x = Math::Clamp(x, 0, bloc_w_db - 2);
		y = Math::Clamp(y, 0, bloc_h_db - 2);
		float h = h_map(x, y);
		if (x < bloc_w_db - 2)	h = Math::Max(h, h_map(x + 1, y));
        if (y < bloc_h_db - 2)
		{
			h = Math::Max(h, h_map(x, y + 1));
            if (x < bloc_w_db - 2)
				h = Math::Max(h, h_map(x + 1, y + 1));
		}
		return h;
	}


	float MAP::get_max_rect_h(int x,int y, int w, int h) const
	{
		const int x1 = Math::Clamp(x - (w>>1), 0, bloc_w_db - 2);
		const int x2 = Math::Clamp(x1 + w, 0, bloc_w_db - 2);
		const int y1 = Math::Clamp(y - (h>>1), 0, bloc_h_db - 2);
		const int y2 = Math::Clamp(y1 + h, 0, bloc_h_db - 2);
		float max_h = h_map(x1, y1);
		for (int Y = y1 ; Y <= y2 ; ++Y)
			for (int X = x1 ; X <= x2 ; ++X)
				max_h = std::max(max_h, h_map(X, Y));
		return max_h;
	}

    void MAP::obstaclesRect(int x1, int y1, int w, int h, bool b, const QByteArray &yardmap, bool open)
	{
        if (yardmap.isEmpty())
		{
			const int x2 = Math::Min(x1 + w, bloc_w_db);
			const int y2 = Math::Min(y1 + h, bloc_h_db);
			y1 = Math::Max(y1, 0);
			x1 = Math::Max(x1, 0);
			for(int y = y1 ; y < y2 ; ++y)
				for(int x = x1 ; x < x2 ; ++x)
					obstacles(x,y) = b;
		}
		else
		{
			const int x2 = Math::Min(x1 + w, bloc_w_db);
			const int y2 = Math::Min(y1 + h, bloc_h_db);
			const int oy1 = y1;
			const int ox1 = x1;
			y1 = Math::Max(y1, 0);
			x1 = Math::Max(x1, 0);
			uint32 i = (y1 - oy1) * w + x1 - ox1;
			const uint32 s = w - (x2 - x1);
			const uint32 l = yardmap.size();
			for(int y = y1 ; y < y2 && l > i ; ++y)
			{
				for(int x = x1 ; x < x2 && l > i ; ++x, ++i)
				{
                    switch(yardmap[i])
					{
					case 'G':
					case 'o':
					case 'w':
					case 'f':
						obstacles(x,y) = b;
						break;
					case 'c':
					case 'C':
						if (!open)
							obstacles(x,y) = b;
						break;
					case 'O':
						if (open)
							obstacles(x,y) = b;
						break;
					};
				}
				i += s;
			}
		}
	}

    void MAP::rect(int x1, int y1, int w, int h, int c, const QByteArray &yardmap, bool open)
	{
        if (yardmap.isEmpty())
		{
			const int y2 = std::min(y1 + h, bloc_h_db - 1);
			const int x2 = std::min(x1 + w, bloc_w_db - 1);
			y1 = std::max(y1, 0);
			x1 = std::max(x1, 0);
			if (y2 <= y1 || x2 <= x1)
				return;
			pMutex.lock();
			for(int y = y1 ; y < y2 ; ++y)
				for(int x = x1 ; x < x2 ; ++x)
					map_data(x, y).unit_idx = c;
			pMutex.unlock();
		}
		else
		{
			uint32 i = 0;
			const int y2 = std::min(y1 + h, bloc_h_db - 1);
			const int x2 = std::min(x1 + w, bloc_w_db - 1);
			if (y1 < 0)	i -= y1 * w;
			if (x1 < 0)	i -= x1;
			y1 = std::max(y1, 0);
			x1 = std::max(x1, 0);
			if (y2 <= y1 || x2 <= x1)
				return;

			const int dw = w - (x2 - x1);
			const unsigned int l = yardmap.size();
			pMutex.lock();
			for (int y = y1; y < y2; ++y)
			{
				for (int x = x1; x < x2; ++x)
				{
					if (l <= i)
					{
						pMutex.unlock();
						return;
					}
                    switch(yardmap[i])
					{
					case 'G':
					case 'o':
					case 'w':
					case 'f':
						map_data(x, y).unit_idx = c;
						break;
					case 'c':
					case 'C':
						if (!open)
							map_data(x, y).unit_idx = c;
						break;
					case 'O':
						if (open)
							map_data(x, y).unit_idx = c;
						break;
					};
					++i;
				}
				i += dw;
			}
			pMutex.unlock();
		}
	}


	bool MAP::check_rect(int x1, int y1, int w, int h, const int c) const
	{
		if (w == 0 || h == 0)
			return true;
		const int y2 = std::min(y1 + h, bloc_h_db - 1);
		const int x2 = std::min(x1 + w, bloc_w_db - 1);
		y1 = std::max(y1, 0);
		x1 = std::max(x1, 0);
		if (y2 <= y1 || x2 <= x1)	return false;
		for(int y = y1 ; y < y2 ; ++y)
			for(int x = x1 ; x < x2 ; ++x)
			{
				const int idx = map_data(x, y).unit_idx;
				if (idx != c && idx != -1)
					return false;
			}
		return true;
	}

	bool MAP::check_rect_discovered(int x1,int y1,int w,int h,const int c) const		// Check if the area has been fully discovered
	{
		const int y2 = std::min((y1 + h + 1) >> 1, bloc_h - 1);
		const int x2 = std::min((x1 + w + 1) >> 1, bloc_w - 1);
		x1 >>= 1;
		y1 >>= 1;
		y1 = std::max(y1, 0);
		x1 = std::max(x1, 0);
		if (y2 <= y1 || x2 <= x1)
			return false;
		for(int y = y1 ; y < y2 ; ++y)
			for(int x = x1 ; x < x2 ; ++x)
				if (!(view_map(x,y) & c))
					return false;
		return true;
	}


	float MAP::check_rect_dh(int x1,int y1,int w,int h) const
	{
		const int y2 = std::min(y1 + h, bloc_h_db - 1);
		const int x2 = std::min(x1 + w, bloc_w_db - 1);
		y1 = std::max(y1, 0);
		x1 = std::max(x1, 0);
		if (y2 <= y1 || x2 <= x1)	return 0.0f;
		float max_dh = 0.0f;
		bool on_water = false;
		for (int y = y1 ; y < y2 ; ++y)
			for (int x = x1 ; x < x2 ; ++x)
			{
				max_dh = Math::Max(max_dh, slope(x,y));
				on_water |= map_data(x, y).isUnderwater();
			}
		if (on_water)
			max_dh = -max_dh;
		return max_dh;
	}

	float MAP::check_max_depth(int x1,int y1,int w,int h) const
	{
		const int y2 = std::min(y1 + h, bloc_h_db - 1);
		const int x2 = std::min(x1 + w, bloc_w_db - 1);
		y1 = std::max(y1, 0);
		x1 = std::max(x1, 0);
		if (y2 <= y1 || x2 <= x1)	return 0.0f;
		float depth = -sealvl;
		for(int y = y1 ; y < y2 ; ++y)
			for(int x = x1 ; x < x2 ; ++x)
				depth = std::max(depth, -h_map(x, y));
		return depth + sealvl;
	}


	float MAP::check_min_depth(int x1, int y1, int w, int h) const
	{
		const int y2 = std::min(y1 + h, bloc_h_db - 1);
		const int x2 = std::min(x1 + w, bloc_w_db - 1);
		y1 = std::max(y1, 0);
		x1 = std::max(x1, 0);
		if (y2 <= y1 || x2 <= x1)	return 255.0f;
		float depth = 255.0f - sealvl;
		for(int y = y1 ; y < y2 ; ++y)
			for(int x = x1 ; x < x2 ; ++x)
				depth = std::min(depth, -h_map(x, y));
		return depth + sealvl;
	}


    bool MAP::check_vents(int x1, int y1, int w, int h, const QByteArray &yard_map) const
	{
        if (yard_map.isEmpty())
			return true;
		const int y2 = std::min(y1 + h, bloc_h_db - 1);
		const int x2 = std::min(x1 + w, bloc_w_db - 1);
		y1 = std::max(y1, 0);
		x1 = std::max(x1, 0);

		if (y2 <= y1 || x2 <= x1)
			return false;

		const int dw = w - (x2-x1);
		const unsigned int l = yard_map.size();
		int i = 0;
		bool ok = true;
		for (int y = y1 ; y < y2 ; ++y)
		{
			for (int x = x1 ; x < x2 ; ++x)
			{
				if (l <= (unsigned int)i)
					return ok;
				if (yard_map[i] == 'G')
				{
					ok = false;
					const int feature_id = map_data(x, y).stuff;
					if (feature_id >= 0)
					{
						const Feature* const pFeature = feature_manager.getFeaturePointer(features.feature[feature_id].type);
                        if (pFeature && pFeature->geothermal)
							return true;
					}
				}
				++i;
			}
			i += dw;
		}
		return ok;
	}


	bool MAP::check_lava(int x1, int y1, int w, int h) const
	{
		const int y2 = std::min(y1 + h, bloc_h_db - 1);
		const int x2 = std::min(x1 + w, bloc_w_db - 1);
		y1 = std::max(y1, 0);
		x1 = std::max(x1, 0);

		if (y2 <= y1 || x2 <= x1)
			return false;
		for (int y = y1 ; y < y2 ; ++y)
		{
			for (int x = x1 ; x < x2 ; ++x)
			{
				if (bloc[bmap(x, y)].lava)
					return true;
			}
		}
		return false;
	}



	void MAP::clean_map()		// Used to remove all objects when loading a saved game
	{
		for (int y = 0 ; y < bloc_h_db ; ++y)
		{
			for (int x = 0 ; x < bloc_w_db ; ++x)
			{
				map_data(x, y).stuff = -1;
				map_data(x, y).unit_idx = -1;
			}
		}
	}

	void MAP::destroy()
	{
		/*------------- Experimental: code for new map format -----------------------*/
		/*	if (macro_bloc) {
			for(uint32 y=0;y<macro_h;y++)
			for(uint32 x=0;x<macro_w;x++)
			macro_bloc[y][x].destroy();
			DELETE_ARRAY(macro_bloc[0]);
			DELETE_ARRAY(macro_bloc);
			macro_w=0;
			macro_h=0;
			}*/
		/*---------------------------------------------------------------------------*/

		view_map.resize(0,0);
		sight_map.resize(0,0);
		radar_map.resize(0,0);
		sonar_map.resize(0,0);

        detail_shader = nullptr;
        shadow2_shader = nullptr;
        details_tex = nullptr;

		DELETE_ARRAY(low_vtx);
		DELETE_ARRAY(low_vtx_flat);
		DELETE_ARRAY(low_tcoord);
		DELETE_ARRAY(low_col);
		DELETE_ARRAY(low_index);
        low_tex = nullptr;

		ota_data.destroy();
        lava_map = nullptr;
		view.resize(0, 0);
		map_data.resize(0, 0);
		ph_map.resize(0, 0);
		h_map.resize(0, 0);
		bmap.resize(0, 0);
        tex.clear();
		if (lvl)
		{
			for(int i = 0 ; i < bloc_h * bloc_w ; i++)
				DELETE_ARRAY(lvl[i]);
			DELETE_ARRAY(lvl);
		}
		if (bloc && nbbloc>0)
		{
			for(int i = 0 ; i < nbbloc ; i++)
			{
				bloc[i].point = NULL;
				bloc[i].destroy();
			}
			DELETE_ARRAY(bloc);
		}
        mini = QImage();
		init();
        detail_shader = nullptr;		// Because init will reload it

		the_map = NULL;
	}

	void MAP::clear_FOW( sint8 FOW_flags )
	{
		if (FOW_flags < 0)	FOW_flags = fog_of_war;
		fog_of_war = FOW_flags;

		if (fog_of_war & FOW_BLACK)
			view_map.clear(0);
		else
			view_map.clear(0xFF);
		if (fog_of_war & FOW_GREY)
			sight_map.clear(0);
		else
			sight_map.clear(0xFF);

		if (fog_of_war == FOW_DISABLED)
		{
			radar_map.clear(0xFF);
			sonar_map.clear(0xFF);
		}
	}

	void MAP::load_details_texture( const QString &filename )
	{
        QImage tex = gfx->load_image(filename);
        if (!tex.isNull())
		{
			uint32 average = 0;
# ifndef TA3D_PLATFORM_DARWIN
            switch(tex.depth())
            {
            case 8:
                for (int y = 0; y < tex.height(); ++y)
                    for (int x = 0; x < tex.width(); ++x)
                        average += SurfaceByte(tex, x, y) * 3;
                break;
            case 24:
                for (int y = 0; y < tex.height(); ++y)
                {
                    for (int x = 0; x < tex.width(); ++x)
                    {
                        average += SurfaceByte(tex, x * 3, y);
                        average += SurfaceByte(tex, x * 3 + 1,y);
                        average += SurfaceByte(tex, x * 3 + 2,y);
                    }
                }
                break;
            case 32:
                for (int y = 0; y < tex.height(); ++y)
                {
                    for (int x = 0; x < tex.width(); ++x)
                    {
                        average += SurfaceByte(tex, (x << 2), y);
                        average += SurfaceByte(tex, (x << 2) + 1,y);
                        average += SurfaceByte(tex, (x << 2) + 2,y);
                    }
                }
                break;
            };
            average /= tex.width() * tex.height() * 3;
# endif
            if (!average)
				average = 1;
			color_factor = 255.0f / (float)average;
			details_tex = gfx->make_texture(tex, FILTER_TRILINEAR, false);
		}
		else
		{
			details_tex = 0;
			color_factor = 1.0f;
		}
	}


	void MAP_OTA::load(const QString& filename)
	{
        QIODevice *file = VFS::Instance()->readFile(filename);
		if (file)
		{
			load(file);
			delete file;
		}
	}


    void MAP_OTA::load(QIODevice *file)
	{
		destroy();

		TDFParser parser;
        const QByteArray &buffer = file->readAll();
        parser.loadFromMemory("OTA", buffer.data(), buffer.size(),false,true,false);
		file->close();

		missionname = parser.pullAsString("GlobalHeader.missionname");
		planet = parser.pullAsString("GlobalHeader.planet");
		glamour = parser.pullAsString("GlobalHeader.glamour");
		missiondescription = parser.pullAsString("GlobalHeader.missiondescription");
		tidalstrength = parser.pullAsInt("GlobalHeader.tidalstrength");
		solarstrength = parser.pullAsInt("GlobalHeader.solarstrength");
		lavaworld = parser.pullAsBool("GlobalHeader.lavaworld");
		killmul = (short)parser.pullAsInt("GlobalHeader.killmul", 50);
		minwindspeed = parser.pullAsInt("GlobalHeader.minwindspeed");
		maxwindspeed = parser.pullAsInt("GlobalHeader.maxwindspeed");
		gravity = (float)parser.pullAsInt("GlobalHeader.gravity") * 0.1f;
		numplayers = parser.pullAsString("GlobalHeader.numplayers");
		map_size = parser.pullAsString("GlobalHeader.size");
		SurfaceMetal = parser.pullAsInt("GlobalHeader.Schema 0.SurfaceMetal");
		MohoMetal = parser.pullAsInt("GlobalHeader.Schema 0.MohoMetal");
        for(int s = 0 ; parser.exists( QString("globalheader.schema 0.specials.special%1").arg(s) ) ; ++s)
		{
            const QString &key = QString("GlobalHeader.Schema 0.specials.special%1.").arg(s);
            const QString &specialWhat = parser.pullAsString(key + "specialwhat").toLower();
			if (specialWhat.startsWith("startpos"))
			{
                int index = Substr(specialWhat, 8, specialWhat.size() - 8).toInt(nullptr, 0) - 1;
                startX[index] = parser.pullAsInt(key + "xpos");
                startZ[index] = parser.pullAsInt(key + "zpos");
			}
		}
		waterdoesdamage = parser.pullAsBool("GlobalHeader.waterdoesdamage");
		waterdamage = parser.pullAsInt("GlobalHeader.waterdamage");
        const QString &tmp = parser.pullAsString("GlobalHeader.Schema 0.type").toLower();
		network = tmp.startsWith("network");
		// Special flag for slate world which has a white fog background
        whitefog = planet.toUpper() == "SLATE";

		if (waterdamage == 0)
			waterdoesdamage=false;
	}

	void MAP::draw_mini(int x1,int y1,int w,int h, Camera* cam, byte player_mask)			// Draw the mini-map
	{
        if (mini.isNull())
			return;		// Check if it exists

		gfx->set_color(0xFFFFFFFF);

		const int rw = w * mini_w / 252;
		const int rh = h * mini_h / 252;
		x1 += (w - rw) >> 1;
		y1 += (h - rh) >> 1;
		const float lw = (float)mini_w / 252.0f;
		const float lh = (float)mini_h / 252.0f;
		gfx->drawtexture(glmini, (float)x1, (float)y1, (float)x1 + (float)rw, (float)y1 + (float)rh, 0.0f, 0.0f, lw, lh);

		if (rh == 0 || rw == 0) return;

		if (fog_of_war != FOW_DISABLED)
		{
			glEnable( GL_BLEND );
			glBlendFunc( GL_ZERO, GL_SRC_COLOR );			// Special blending function
			glDisable( GL_TEXTURE_2D );

			int MY = 0;
			const int DY = 0x10000 * ( bloc_h_db - 2 ) / rh;
			const int DX = 0x10000 * ( bloc_w_db - 2 ) / rw;

			std::vector<Vector2D> lines;
			std::vector<uint32> colors;
			uint32 col = 0U;

			gfx->lock();

			for (int y = 0; y < rh; ++y)
			{
				const int my = MY >> 17;
				MY += DY;
				int old_col = -1;
				int old_x = -1;
				int MX = 0;
				for (int x = 0 ; x < rw; ++x)
				{
					const int mx = MX >> 17;
					MX += DX;
					if (!(view_map(mx,my) & player_mask))
					{
						if (old_col != 0 )
						{
							if (old_x != -1 )
							{
								lines.push_back( Vector2D(float(x1 + old_x), float(y1 + y)) );
								lines.push_back( Vector2D(float(x1 + x), float(y1 + y)) );
								colors.push_back(col);
								colors.push_back(col);
							}
							col = 0x00000000U;
							old_col = 0;
							old_x = x;
						}
					}
					else
					{
						if (!(sight_map(mx,my) & player_mask))
						{
							if (old_col != 1)
							{
								if (old_x != -1)
								{
									lines.push_back( Vector2D(float(x1 + old_x), float(y1 + y)) );
									lines.push_back( Vector2D(float(x1 + x), float(y1 + y)) );
									colors.push_back(col);
									colors.push_back(col);
								}
								col = 0x7F7F7F7FU;
								old_x = x;
								old_col = 1;
							}
						}
						else
						{
							if (old_col != 2)
							{
								if (old_x != -1)
								{
									lines.push_back( Vector2D(float(x1 + old_x), float(y1 + y)) );
									lines.push_back( Vector2D(float(x1 + x), float(y1 + y)) );
									colors.push_back(col);
									colors.push_back(col);
								}
								old_x = -1;
								old_col = 2;
							}
						}
					}
				}
				if (old_x != -1)
				{
					lines.push_back( Vector2D(float(x1 + old_x), float(y1 + y)) );
					lines.push_back( Vector2D(float(x1 + rw), float(y1 + y)) );
					colors.push_back(col);
					colors.push_back(col);
				}
			}

			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_COLOR_ARRAY);
			glVertexPointer(2, GL_FLOAT, 0, &(lines.front()));
			glColorPointer(4, GL_UNSIGNED_BYTE, 0, &(colors.front()));

			glDrawArrays(GL_LINES, 0, (GLsizei)lines.size());

			glDisableClientState(GL_COLOR_ARRAY);
			glDisable(GL_BLEND);

			gfx->unlock();
		}

		if (!cam)
			return;

		Vector3D P;
		Vector3D A, B, C, D;
		Vector3D PA, PB, PC, PD;

		if (lp_CONFIG->ortho_camera)
		{
			A = B = C = D = cam->dir;
			PA = cam->pos + cam->zoomFactor * ( -(float)gfx->SCREEN_W_HALF * cam->side - (float)gfx->SCREEN_H_HALF * cam->up );
			PB = cam->pos + cam->zoomFactor * (  (float)gfx->SCREEN_W_HALF * cam->side - (float)gfx->SCREEN_H_HALF * cam->up );
			PC = cam->pos + cam->zoomFactor * (  (float)gfx->SCREEN_W_HALF * cam->side + (float)gfx->SCREEN_H_HALF * cam->up );
			PD = cam->pos + cam->zoomFactor * ( -(float)gfx->SCREEN_W_HALF * cam->side + (float)gfx->SCREEN_H_HALF * cam->up );
		}
		else
		{
			A = cam->dir + cam->widthFactor * (-cam->side) - 0.75f * cam->up;
			B = cam->dir + cam->widthFactor * (cam->side)  - 0.75f * cam->up;
			C = cam->dir + cam->widthFactor * (cam->side)  + 0.75f * cam->up;
			D = cam->dir + cam->widthFactor * (-cam->side) + 0.75f * cam->up;
			PA = PB = PC = PD = cam->pos;
		}
		const int nmax = 64;
		float cx[4 * nmax + 4], cy[4 * nmax + 4];
		if (A.y < 0.0f)
		{	P = PA + PA.y / fabsf(A.y) * A;	cx[0] = P.x;	cy[0] = P.z; }
		else
		{	P = PA + 10000.0f * A;	cx[0] = P.x;	cy[0] = P.z; }
		if (B.y < 0.0f)
		{	P = PB + PB.y / fabsf(B.y) * B;	cx[1] = P.x;	cy[1] = P.z; }
		else
		{	P = PB + 10000.0f * B;	cx[1] = P.x;	cy[1] = P.z; }
		if (C.y < 0.0f)
		{	P = PC + PC.y / fabsf(C.y) * C;	cx[2] = P.x;	cy[2] = P.z; }
		else
		{	P = PC + 10000.0f * C;	cx[2] = P.x;	cy[2] = P.z; }
		if (D.y < 0.0f)
		{	P = PD + PD.y / fabsf(D.y) * D;	cx[3] = P.x;	cy[3] = P.z; }
		else
		{	P = PD + 10000.0f * D;	cx[3] = P.x;	cy[3] = P.z; }

		for (int i = 0; i < 4; ++i)
		{
			cx[i] = (cx[i] + 0.5f * (float)map_w) * (float)rw / (float)map_w;
			cy[i] = (cy[i] + 0.5f * (float)map_h) * (float)rh / (float)map_h;
		}
		for (int i = 0; i < 4; ++i)
		{
			for (int e = 0; e < nmax; ++e)
			{
				cx[i * nmax + e + 4] = (cx[i] * float(nmax - e) + cx[(i + 1) % 4] * float(e + 1)) / float(nmax + 1);
				cy[i * nmax + e + 4] = (cy[i] * float(nmax - e) + cy[(i + 1) % 4] * float(e + 1)) / float(nmax + 1);
			}
		}
		for (int i = 0; i < 4 + (nmax << 2); ++i)
		{
			if (cx[i] < 0.0f)
				cx[i] = 0.0f;
			else
				if (cx[i] > rw)
					cx[i] = (float)rw;
			if (cy[i] < 0.0f)
				cy[i] = 0.0f;
			else
				if (cy[i] > rh)
					cy[i] = (float)rh;
		}

		glDisable(GL_TEXTURE_2D);
		glColor3ub(0xE5, 0xE5, 0x66);
		glBegin(GL_LINE_LOOP);
		for (int i = 0; i < 4; ++i)
		{
			glVertex2f(cx[i] + (float)x1,  cy[i] + (float)y1);
			for (int e = 0; e < nmax; ++e)
				glVertex2f((float)x1 + cx[i * nmax + e + 4],  (float)y1 + cy[i * nmax + e + 4]);
		}
		glEnd();
		glColor3ub(0xFF,0xFF,0xFF);
		glEnable(GL_TEXTURE_2D);
	}



	void MAP::update_player_visibility( int player_id, int px, int py, int r, int rd, int sn, int rd_j, int sn_j, bool jamming, bool black )
	{
		gfx->lock();

		px >>= 1;
		py >>= 1;

		// Update jamming maps
		if (jamming)
		{
			if (rd_j > 0)
				radar_map.circlefill(px, py, rd_j, 0);
			if (sn_j > 0)
				sonar_map.circlefill(px, py, sn_j, 0);
		}
		else
		{
			const byte mask = byte(1 << player_id);
			const int r2 = r * r;
			const int rd2 = rd * rd;
			const int sn2 = sn * sn;
			// Update detector maps
			if (sn > 0)
				for (int y = 0; y <= sn; ++y) // Update sonar data
				{
					const int x = (int)(0.5f + std::sqrt((float)(sn2 - y * y)));
					int ry = py - y;
					if (ry >= 0 && ry < sonar_map.getHeight())
					{
						int rx = std::max<int>(0, px - x);
						const int lx = std::min<int>(px + x, sonar_map.getWidth() - 1);
						for( ; rx <= lx ; ++rx)
							sonar_map(rx, ry) |= mask;
					}
					if (y != 0)
					{
						ry = py + y;
						if (ry >= 0 && ry < sonar_map.getHeight())
						{
							int rx = std::max<int>(0, px - x);
							const int lx = std::min<int>(px + x, sonar_map.getWidth() - 1);
							for( ; rx <= lx ; ++rx)
								sonar_map(rx, ry) |= mask;
						}
					}
				}
			if (rd > 0 )
				for (int y = 0; y <= rd; ++y) // Update radar data
				{
					const int x = (int)(0.5f + std::sqrt((float)(rd2 - y * y)));
					int ry = py - y;
					if (ry >= 0 && ry < radar_map.getHeight())
					{
						int rx = std::max<int>(0, px - x);
						const int lx = std::min<int>(px + x, radar_map.getWidth() - 1);
						for( ; rx <= lx ; ++rx)
							radar_map(rx, ry) |= mask;
					}
					if (y != 0)
					{
						ry = py + y;
						if (ry >= 0 && ry < radar_map.getHeight())
						{
							int rx = std::max<int>(0, px - x);
							const int lx = std::min<int>(px + x, radar_map.getWidth() - 1);
							for( ; rx <= lx ; ++rx)
								radar_map(rx,ry) |= mask;
						}
					}
				}
			if (fog_of_war & FOW_GREY)
				for(int y = 0; y <= r; ++y) // Update view data
				{
					const int x = (int)(0.5f + std::sqrt((float)(r2 - y * y)));
					int ry = py - y;
					if (ry >= 0 && ry < sight_map.getHeight())
					{
						int rx = std::max<int>(0, px - x);
						const int lx = std::min<int>(px + x, sight_map.getWidth() - 1);
						for( ; rx <= lx ; ++rx)
							sight_map(rx,ry) |= mask;
					}
					if (y != 0)
					{
						ry = py + y;
						if (ry >= 0 && ry < sight_map.getHeight())
						{
							int rx = std::max<int>(0, px - x);
							const int lx = std::min<int>(px + x, sight_map.getWidth() - 1);
							for( ; rx <= lx ; ++rx)
								sight_map(rx, ry) |= mask;
						}
					}
				}
			if (black && (fog_of_war & FOW_BLACK))
				for (int y = 0; y <= r; ++y) // Update view data
				{
					const int x = (int)(0.5f + std::sqrt((float)(r2 - y * y)));
					int ry = py - y;
					if (ry >= 0 && ry < view_map.getHeight())
					{
						int rx = std::max<int>(0, px - x);
						const int lx = std::min<int>(px + x, view_map.getWidth() - 1);
						for( ; rx <= lx ; ++rx)
							view_map(rx, ry) |= mask;
					}
					if (y != 0)
					{
						ry = py + y;
						if (ry >= 0 && ry < view_map.getHeight())
						{
							int rx = std::max<int>(0, px - x);
							const int lx = std::min<int>(px + x, view_map.getWidth() - 1);
							for( ; rx <= lx ; ++rx)
								view_map(rx, ry) |= mask;
						}
					}
				}
		}
		gfx->unlock();
	}

	std::vector<Vector3D> MAP::get_visible_volume() const
	{
		std::vector<Vector3D>  volume;
		const bool bFarSight = lp_CONFIG->far_sight && !Camera::inGame->mirror;
		const float zfar = Camera::inGame->zfar;
		const float cam_h = Camera::inGame->rpos.y - get_unit_h(Camera::inGame->rpos.x, Camera::inGame->rpos.z);
		const float map_zfar = 600.0f + Math::Max((cam_h - 150.0f) * 2.0f, 0.0f);
		if (bFarSight)
			Camera::inGame->zfar = map_zfar;
		Camera::inGame->getFrustum(volume);
		if (bFarSight)
			Camera::inGame->zfar = zfar;
		Vector3D dir;
		for(int i = 4 ; i < 8; ++i)
		{
			dir = volume[i] - volume[i-4];
			const float dist_max = dir.norm();
			dir = 1.0f / dist_max * dir;;
			if (dir.y > 0.0f)       // Heading up
			{
				volume[i] += (512.0f * H_DIV - volume[i].y) * dir;
				continue;
			}
			const Vector3D map_hit = hit( volume[i-4], dir, false, dist_max, true) + 30.0f * dir;
			if ( (map_hit - volume[i-4]) % dir < dist_max)
				volume[i] = map_hit;
		}
		return volume;
	}

	inline float sq( float a )	{	return a * a;	}


	void MAP::draw(Camera* cam,byte player_mask,bool FLAT,float niv,float t,float dt,bool depth_only,bool check_visibility,bool draw_uw)
	{
		if (FLAT && !water)
			return;

		bool low_def_view = cam->rpos.y > gfx->low_def_limit;		// Low detail map for mega zoom

		if (low_def_view)
		{
			cam->zfar = sqrtf( float(map_w * map_w + map_h * map_h) + cam->rpos.y * cam->rpos.y);      // We want to see everything
            cam->setView(true);
			draw_LD(player_mask, FLAT, niv, t);

			view.clear(1);
			ox1 = 0;
			ox2 = bloc_w - 1;
			oy1 = 0;
			oy2 = bloc_h - 1;
		}
		else
		{
			const bool bFarSight = lp_CONFIG->far_sight && !cam->mirror;
			const float zfar = cam->zfar;
			const float cam_h = cam->rpos.y - get_unit_h(cam->rpos.x, cam->rpos.z);
			const float map_zfar = 600.0f + Math::Max((cam_h - 150.0f) * 2.0f, 0.0f);
			if (bFarSight)      // Far sight mode: renders low definition map under the HD version in order to show the whole map at the horizon
			{
				cam->setView(true);
				GLdouble eq[] = { cam->dir.x, cam->dir.y, cam->dir.z, -map_zfar + 64.0f - cam->rpos % cam->dir};
				glClipPlane(GL_CLIP_PLANE3, eq);
				glEnable(GL_CLIP_PLANE3);

				draw_LD(player_mask, FLAT, niv, t);

				glDisable(GL_CLIP_PLANE3);
			}

            cam->setView(lp_CONFIG->shadow_quality < 2 || FLAT);
			if (lp_CONFIG->far_sight)
				cam->zfar = map_zfar;

			draw_HD(cam, player_mask, FLAT, niv, t, dt, depth_only, check_visibility, draw_uw);

			if (lp_CONFIG->far_sight)
				cam->zfar = zfar;
		}
	}




	void MAP::draw_LD(byte player_mask,bool FLAT,float niv,float t)
	{
		gfx->lock();

		glPushMatrix();

		if (FLAT)
			glTranslatef(0.0f, 0.0f, sea_dec);

		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);
        if (!FLAT && !tex.empty())
			gfx->ReInitAllTex( true );
		if (!FLAT)
			glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

		if (FLAT)
			glTranslatef(cosf(t), 0.0f, sinf(t));

		int i = 0;
		for (int y = 0; y <= low_h; ++y)
		{
			int Y = y * (bloc_h_db - 2) / low_h;
			for (int x = 0; x <= low_w; ++x)
			{
				int X = x * (bloc_w_db - 2) / low_w;
				int Z = Y + get_zdec_notest(X,Y);
				if (Z >= bloc_h_db - 1)
					Z = bloc_h_db - 2;
				if (!(view_map(X >> 1, Z >> 1) & player_mask))
					low_col[i << 2] = low_col[(i << 2) + 1] = low_col[(i << 2) + 2] = low_col[(i << 2) + 3] = 0;
				else
				{
					low_col[(i << 2) + 3] = 255;
					if (!(sight_map(X >> 1, Z >> 1) & player_mask))
						low_col[i << 2] = low_col[(i << 2) + 1] = low_col[(i << 2) + 2] = 127;
					else
						low_col[i << 2] = low_col[(i << 2) + 1] = low_col[(i << 2) + 2] = 255;
				}
				++i;
			}
		}

		glDisableClientState(GL_NORMAL_ARRAY);		// we don't need normal data
		glEnableClientState(GL_COLOR_ARRAY);        // Colors are used to render fog of war
		glColorPointer(4,GL_UNSIGNED_BYTE,0,low_col);
		glEnableClientState(GL_VERTEX_ARRAY);		// vertex coordinates
		if (FLAT)
		{
			glTranslatef(0.0f, niv, 0.0f);
			glVertexPointer( 3, GL_FLOAT, 0, low_vtx_flat);
		}
		else
			glVertexPointer( 3, GL_FLOAT, 0, low_vtx);

		glClientActiveTextureARB(GL_TEXTURE0_ARB );
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, low_tcoord);

		glActiveTextureARB(GL_TEXTURE0_ARB);
		glEnable(GL_TEXTURE_2D);
        low_tex->bind();

		glDrawRangeElements(GL_TRIANGLE_STRIP, 0, (low_w+1)*(low_h+1)-1, low_nb_idx,GL_UNSIGNED_INT,low_index);		// draw this map

		glDisableClientState(GL_COLOR_ARRAY);

		glPopMatrix();

		gfx->unlock();
	}

	void renderLine(std::vector<int> &xMin, std::vector<int> &xMax, int x0, int y0, int x1, int y1, int xmax)
	{
		if (y0 == y1)               // We don't need to handle this case here
			return;
		if (y0 > y1)                // We want y0 < y1
		{
			x0 ^= x1;   x1 ^= x0;   x0 ^= x1;
			y0 ^= y1;   y1 ^= y0;   y0 ^= y1;
		}
		int ymax = Math::Min((int)y1, (int)xMin.size() - 1);
		for(int y = Math::Max(y0, 0) ; y <= ymax ; y++)
		{
			int x = x0 + (y - y0) * (x1 - x0) / (y1 - y0);
			if (x < 0)
				x = 0;
			else if (x > xmax)
				x = xmax;
			if (xMin[y] > x || xMin[y] == -1)
				xMin[y] = x;
			if (xMax[y] < x || xMax[y] == -1)
				xMax[y] = x;
		}
	}



	void MAP::draw_HD(Camera* cam,byte player_mask,bool FLAT,float niv,float t,float dt,bool depth_only,bool check_visibility,bool draw_uw)
	{
		glPushMatrix();

		gfx->lock();
		if (FLAT)
			glTranslatef(0.0f, 0.0f, sea_dec);

		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);
		if (!FLAT)
		{
            if (!tex.empty())
				gfx->ReInitAllTex(true);
			else
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}

		// ------------------------------------------------------------------
		// Beginning of visible area calculations
		// ------------------------------------------------------------------

		// First we need the frustum volume
		static std::vector<Vector3D> frustum;
		frustum.clear();
		cam->getFrustum(frustum);

		const int nLinks = 18;
		const int fLink[][2] = { {0, 1}, {1, 2}, {2, 3}, {0, 2}, {1, 3},     // Front face
								{4, 5}, {5, 6}, {6, 7}, {4, 6}, {5, 7},     // Back face
								{0, 4}, {1, 5}, {2, 6}, {3, 7},             // Sides
								{0, 6}, {1, 7}, {0, 5}, {2, 7} };           // NB: fLink[i][0] < fLink[i][1] for all possible i

		// Once we have the frustum volume, we compute the intersection between this volume and the map "plane" (y = ymin, then y = ymax)
		// So we need to know what is below yref and what is above (maybe we'll have nothing to render)
		std::vector<int> xMin;      // Those vectors will store the visible lines
		std::vector<int> xMax;
		xMin.resize(bloc_h, -1);    // Initializing values to -1 ensure we won't render blank lines
		xMax.resize(bloc_h, -1);
		int n = 0;
		int x1 = bloc_w - 1;
		int y1 = bloc_h - 1;
		int x2 = 0;
		int y2 = 0;
		float ymin = 0.0f;
		float ymax = 0.0f;
		for (unsigned int i = 0 ; i < frustum.size(); ++i)
		{
			const int x = (int)((frustum[i].x + (float)map_w_d) * 0.0625f);
			const int y = (int)((frustum[i].z + (float)map_h_d) * 0.0625f);
			x1 = Math::Min(x1, x);
			y1 = Math::Min(y1, y);
			x2 = Math::Max(x2, x);
			y2 = Math::Max(y2, y);
			if (i == 0)
				ymin = ymax = frustum[i].y;
			ymin = Math::Min(ymin, frustum[i].y);
			ymax = Math::Max(ymax, frustum[i].y);
		}
		ymin = Math::Max(ymin, 0.0f);
		ymax = Math::Min(ymax, 255.0f * H_DIV);
		{
			const float my = ymin;
			const float My = ymax;
			ymin = 0.9f * my + 0.1f * My;
			ymax = 0.1f * my + 0.9f * My;
		}
		for (int yRefIndex = 0; yRefIndex < 2; ++yRefIndex)
		{
			float yref = 0.;        // We must cover the extreme values to ensure everything that should be rendered will be rendered
			switch(yRefIndex)
			{
				case 0: yref = ymin; break;
				case 1: yref = ymax; break;
			}
			bool bFrustum[8];
			for (unsigned int i = 0 ; i < frustum.size(); ++i)
			{
				if ((bFrustum[i] = (frustum[i].y > yref)))
					++n;
			}
			if (!n)
				continue;

			bool bLink[18];
			for(int i = 0 ; i < nLinks ; i++)               // Detect intersection of links and map plane
				bLink[i] = bFrustum[fLink[i][0]] ^ bFrustum[fLink[i][1]];
            float zOffset = yref * tnt_transform * H_DIV;
			for (int i = 0 ; i < nLinks; ++i)
			{
				if (bLink[i])
				{
					Vector3D A = frustum[ fLink[i][0] ];
					Vector3D B = frustum[ fLink[i][1] ];
					if (fabsf(A.y - B.y) < 0.1f)
						continue;
					Vector3D I = A + (yref - A.y) / (B.y - A.y) * (B - A);
					I.z -= zOffset;
					const int X0 = (int)((I.x + (float)map_w_d) * 0.0625f + 0.5f);
					const int Y0 = (int)((I.z + (float)map_h_d) * 0.0625f + 0.5f);
					x1 = Math::Min(x1, X0);
					y1 = Math::Min(y1, Y0);
					x2 = Math::Max(x2, X0);
					y2 = Math::Max(y2, Y0);
					for(int e = i + 1 ; e < nLinks ; ++e)
					{
						if (fLink[i][0] == fLink[e][0] || fLink[i][0] == fLink[e][1] ||
							fLink[i][1] == fLink[e][0] || fLink[i][1] == fLink[e][1])           // We have detected a border of the visible area
						{
							Vector3D C = frustum[ fLink[e][0] ];
							Vector3D D = frustum[ fLink[e][1] ];
							if (fabsf(C.y - D.y) < 0.1f)
								continue;
							Vector3D J = C + (yref - C.y) / (D.y - C.y) * (D - C);
							J.z -= zOffset;
							const int X1 = (int)((J.x + (float)map_w_d) * 0.0625f + 0.5f);
							const int Y1 = (int)((J.z + (float)map_h_d) * 0.0625f + 0.5f);
							x1 = Math::Min(x1, X1);
							y1 = Math::Min(y1, Y1);
							x2 = Math::Max(x2, X1);
							y2 = Math::Max(y2, Y1);
							renderLine(xMin, xMax, X0, Y0, X1, Y1, bloc_w - 1);
						}
					}
				}
			}
		}

		if (!n) // Nothing to render we can safely give up here
		{
			gfx->unlock();
			glPopMatrix();
			return;
		}

		x1 = Math::Max(x1, 0);
		x2 = Math::Max(x2, 0);
		y1 = Math::Max(y1, 0);
		y2 = Math::Max(y2, 0);
		x1 = Math::Min(x1, bloc_w - 1);
		x2 = Math::Min(x2, bloc_w - 1);
		y1 = Math::Min(y1, bloc_h - 1);
		y2 = Math::Min(y2, bloc_h - 1);

		// ------------------------------------------------------------------
		// End of visible area calculations
		// ------------------------------------------------------------------

		if (!FLAT)
			glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

		Vector3D flat[9];
		if (FLAT)
		{
			flat[0].x = 0.0f;		flat[0].y=niv+cosf(t)*0.5f;			flat[0].z=0.0f;
			flat[1].x = 8.0f;		flat[1].y=niv+cosf(t+1.0f)*0.5f;    flat[1].z=0.0f;
			flat[2].x = 16.0f;		flat[2].y=flat[0].y;				flat[2].z=0.0f;
			flat[3].x = 0.0f;		flat[3].y=niv+cosf(t+1.5f)*0.5f;	flat[3].z=8.0f;
			flat[4].x = 8.0f;		flat[4].y=niv+cosf(t+2.5f)*0.5f;	flat[4].z=8.0f;
			flat[5].x = 16.0f;		flat[5].y=flat[3].y;				flat[5].z=8.0f;
			flat[6].x = 0.0f;		flat[6].y=flat[0].y;				flat[6].z=16.0f;
			flat[7].x = 8.0f;		flat[7].y=flat[1].y;				flat[7].z=16.0f;
			flat[8].x = 16.0f;		flat[8].y=flat[0].y;				flat[8].z=16.0f;
		}

		const bool enable_details = !cam->mirror && (lp_CONFIG->detail_tex || lp_CONFIG->shadow_quality >= 2);

        if (!tex.empty() && !depth_only)
		{
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_2D);
			glClientActiveTextureARB(GL_TEXTURE0_ARB);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);

			if (!FLAT && enable_details)
			{
				glClientActiveTextureARB(GL_TEXTURE1_ARB);
				glActiveTextureARB(GL_TEXTURE1_ARB );
				glEnable(GL_TEXTURE_2D);
                details_tex->bind();
				glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
				glDisable(GL_TEXTURE_GEN_S);
				glDisable(GL_TEXTURE_GEN_T);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				glActiveTextureARB(GL_TEXTURE0_ARB );
			}
		}

		if (FLAT)
			glTranslatef(cosf(t), 0.0f, sinf(t));
        GfxTexture::Ptr old_tex = bloc[0].tex;
		if (!depth_only)
            old_tex->bind();
		if (!FLAT && check_visibility)
		{
			for (int y = oy1; y <= oy2; ++y)
				memset(&(view(ox1, y)), 0, ox2 - ox1 + 1);
			features.list.clear();
			ox1 = x1;	ox2 = x2;
			oy1 = y1;	oy2 = y2;
		}
		else
		{
			if (!check_visibility)
			{
				x1 = ox1;	x2 = ox2;
				y1 = oy1;	y2 = oy2;
				for (std::vector<int>::iterator i = features.list.begin() ; i != features.list.end() ; ++i)
					features.feature[*i].draw = true;
			}
		}
		const int lavaprob = (int)(1000 * dt);
		Vector3D buf_p[4500]; // Tampon qui accumule les blocs pour les dessiner en chaîne
		float	buf_t[9000];
		uint8	buf_c[18000];
		short	buf_size = 0;				// in blocs
		uint16	index_size = 0;
		bool	was_flat = false;
		glDisableClientState(GL_NORMAL_ARRAY);		// we don't need normal data
		glEnableClientState(GL_VERTEX_ARRAY);		// vertex coordinates
		glEnableClientState(GL_COLOR_ARRAY);		// Colors(for fog of war)
		glColorPointer(4,GL_UNSIGNED_BYTE,0,buf_c);
		glVertexPointer( 3, GL_FLOAT, 0, buf_p);

		if (!FLAT && enable_details)
		{
			switch (lp_CONFIG->shadow_quality)
			{
				case 3:
				case 2:
                    shadow2_shader->bind();
                    shadow2_shader->setUniformValue( "coef", color_factor );
                    shadow2_shader->setUniformValue( "details", 1 );
                    shadow2_shader->setUniformValue( "shadowMap", 7 );
                    shadow2_shader->setmat4f( "light_Projection", gfx->shadowMapProjectionMatrix);
					break;
				default:
                    detail_shader->bind();
                    detail_shader->setUniformValue( "coef", color_factor );
                    detail_shader->setUniformValue( "details", 1 );
			}
		}

		glClientActiveTextureARB(GL_TEXTURE0_ARB );
		glTexCoordPointer(2, GL_FLOAT, 0, buf_t);

//#define DEBUG_UNIT_POS
//#define DEBUG_ENERGY

#ifdef DEBUG_ENERGY
		float Emax = energy(10, 10);
		for(int y = 10 ; y < energy.getHeight() - 10 ; ++y)
			for(int x = 10 ; x < energy.getWidth() - 10 ; ++x)
				Emax = Math::Max(Emax, energy(x,y));
		float IEmax = 255.0f / Emax;
#endif

		int	ox = x1;

		Vector3D T;
		Vector3D V;
		for (int y = y1 ; y <= y2 ; ++y) // Balaye les blocs susceptibles d'être visibles pour dessiner ceux qui le sont
		{
			const int pre_y = y << 4;
			const int Y = y << 1;
			const int pre_y2 = y * bloc_w;
			T.x = (float)-map_w_d;
			T.y = 0.0f;
			T.z = float(pre_y - map_h_d);
			buf_size = 0;
			ox = x1;
			bool was_clean = false;

			int rx1 = xMin[y];
			int rx2 = xMax[y];
			if (rx1 == -1 || rx2 == -1)
				continue;
			if (rx1 > 0)
				rx1--;
			if (rx2 < bloc_w - 1)
                rx2++;

			for (int x = rx1 ; x <= rx2 ; ++x)
			{
				int X = x << 1;
				if (!FLAT && check_visibility)
				{
					if (!(view_map(x, y) & player_mask))
					{
						if (water)
						{
							if (map_data(X, Y).isUnderwater() && map_data(X, Y | 1).isUnderwater() && map_data(X | 1, Y).isUnderwater() && map_data(X | 1, Y | 1).isUnderwater())
								view(x, y) = 2;
							else
								view(x, y) = 3;
						}
					}
					else
					{
						if (!(sight_map(x, y) & player_mask))
						{
							if (map_data(X, Y).isUnderwater() || map_data(X, Y | 1).isUnderwater() || map_data(X | 1, Y).isUnderwater() || map_data(X | 1, Y | 1).isUnderwater())
								view(x, y) = 2;
							else
								view(x, y) = 3;
						}
						else
							view(x, y) = 1;

						if (map_data(X, Y).stuff >= 0 && map_data(X, Y).stuff < features.max_features) // Flag are visible objects in that bloc
						{
							if (features.feature[map_data(X, Y).stuff].type < 0)
								map_data(X, Y).stuff = -1;
							else
							{
								features.feature[map_data(X, Y).stuff].draw = true;
								features.feature[map_data(X, Y).stuff].grey = (view(x, y) & 2) == 2;
								features.list.push_back(map_data(X, Y).stuff);
							}
						}
						if (map_data(X | 1, Y).stuff >= 0 && map_data(X | 1, Y).stuff < features.max_features)
						{
							if (features.feature[map_data(X | 1, Y).stuff].type < 0)
								map_data(X | 1, Y).stuff = -1;
							else
							{
								features.feature[map_data(X | 1, Y).stuff].draw = true;
								features.feature[map_data(X | 1, Y).stuff].grey = (view(x, y) & 2) == 2;
								features.list.push_back(map_data(X | 1, Y).stuff);
							}
						}
						if (map_data(X, Y | 1).stuff >= 0 && map_data(X, Y | 1).stuff < features.max_features)
						{
							if (features.feature[map_data(X, Y | 1).stuff].type < 0)
								map_data(X, Y | 1).stuff = -1;
							else
							{
								features.feature[map_data(X, Y | 1).stuff].draw = true;
								features.feature[map_data(X, Y | 1).stuff].grey = (view(x, y) & 2) == 2;
								features.list.push_back(map_data(X, Y | 1).stuff);
							}
						}
						if (map_data(X | 1, Y | 1).stuff >= 0 && map_data(X | 1, Y | 1).stuff < features.max_features)
						{
							if (features.feature[map_data(X | 1, Y | 1).stuff].type < 0)
								map_data(X | 1, Y | 1).stuff = -1;
							else
							{
								features.feature[map_data(X | 1, Y | 1).stuff].draw = true;
								features.feature[map_data(X | 1, Y | 1).stuff].grey = (view(x, y) & 2) == 2;
								features.list.push_back(map_data(X | 1, Y | 1).stuff);
							}
						}
					}
				}
				else
				{
					if (view(x, y) == 0)
						continue;
					if (view(x, y) == 2 && !draw_uw)
						continue;		// Jump this if it is under water and don't have to be drawn
					if (view(x, y) == 3)
						view(x, y) = 2;
					if (view(x, y)==2 && FLAT)
						view(x, y)=0;
					if (cam->mirror && map_data(X, Y).isFlat())
						continue;
				}
				// Si le joueur ne peut pas voir ce morceau, on ne le dessine pas en clair
				T.x += float(x << 4);
				const int i = bmap(x, y);
				if (FLAT)
				{
					bloc[i].point = lvl[pre_y2 + x];
					if (bloc[i].point == NULL || bloc[i].point[0].y < niv || bloc[i].point[1].y < niv || bloc[i].point[2].y < niv ||
						bloc[i].point[3].y < niv || bloc[i].point[4].y < niv || bloc[i].point[5].y < niv ||
						bloc[i].point[6].y < niv || bloc[i].point[7].y < niv || bloc[i].point[8].y < niv)
						bloc[i].point = flat;
					else
					{
						T.x -= float(x << 4);
						continue;
					}
				}
				else
				{
					if (check_visibility)
					{
						const bool under_water = (h_map(X | 1, Y | 1) < sealvl && h_map(X | 1, Y) < sealvl && h_map(X, Y | 1) < sealvl && h_map(X, Y) < sealvl);

						if ((bloc[i].lava || (under_water && ota_data.lavaworld) ) && !ota_data.whitefog && !lp_CONFIG->pause
							&& (Math::RandomTable() % 1000000) <= lavaprob)		// Lava emiting code moved here because of lava effect using fragment program
						{
							Vector3D POS( float((x << 4) - map_w_d) + 8.0f, sealvl - 5.0f, float(pre_y - map_h_d) + 8.0f );
							V.x = float(((int)(Math::RandomTable() % 201)) - 100);
							V.y = float(((int)(Math::RandomTable() % 51)) + 50);
							V.z = float(((int)(Math::RandomTable() % 201)) - 100);
							V.unit();
							particle_engine.emit_lava(POS, V, 1, 10, float(Math::RandomTable() % 1000) * 0.01f + 30.0f);
						}
						else
						{
							if (!map_data( X, Y).isLava() && water && !ota_data.lavaworld && !ota_data.whitefog && !under_water && !lp_CONFIG->pause &&										// A wave
								(h_map(X | 1, Y | 1) < sealvl || h_map(X | 1, Y) < sealvl || h_map(X, Y | 1) < sealvl || h_map(X, Y) < sealvl) &&
								(h_map(X | 1, Y | 1) >= sealvl || h_map(X, Y | 1) >= sealvl || h_map(X | 1, Y) >= sealvl || h_map(X, Y) >= sealvl) &&
								(Math::RandomTable() % 4000) <= lavaprob &&
								(view_map(x, y) & player_mask) && lp_CONFIG->waves)
							{
								Vector3D grad;
								for(int dz = -2 ; dz <= 2 ; ++dz)
								{
									if (Y + dz >= 0 && Y + dz < h_map.getHeight())
										for(int dx = -2 ; dx <= 2 ; ++dx)
										{
											if (X + dx < 0 || X + dx >= h_map.getWidth())
												continue;
											const float v = h_map(X + dx, Y + dz) - sealvl;
											grad.x += (float)dx * std::exp(0.125f * float(dx * dx + dz * dz)) * v;
											grad.z += (float)dz * std::exp(0.125f * float(dx * dx + dz * dz)) * v;
										}
								}
								const float grad_len = grad.sq();
								if (grad_len > 0.0f)
								{
									const Vector3D pos(
											float((x << 4) - map_w_d) + 8.0f,
                                            sealvl + 0.1f,
											float(pre_y - map_h_d) + 8.0f);
                                    grad = (1.0f / sqrtf( grad_len )) * grad;
									fx_manager.addWave(pos - 16.0f * grad, RAD2DEG * std::atan2(-grad.x, -grad.z));
								}
							}
						}
					}
					bloc[i].point = lvl[pre_y2+x];
					if (bloc[i].point == NULL)
					{
						lvl[pre_y2+x] = bloc[i].point = new Vector3D[9];
						if (tnt)
						{
							bloc[i].point[0].x=T.x;			bloc[i].point[0].z=get_zdec(X,Y)+T.z;
							bloc[i].point[1].x=8.0f+T.x;	bloc[i].point[1].z=get_zdec(X|1,Y)+T.z;
							bloc[i].point[2].x=16.0f+T.x;	bloc[i].point[2].z=get_zdec(X+2,Y)+T.z;
							bloc[i].point[3].x=T.x;			bloc[i].point[3].z=8.0f+get_zdec(X,Y|1)+T.z;
							bloc[i].point[4].x=8.0f+T.x;	bloc[i].point[4].z=8.0f+get_zdec(X|1,Y|1)+T.z;
							bloc[i].point[5].x=16.0f+T.x;	bloc[i].point[5].z=8.0f+get_zdec(X+2,Y|1)+T.z;
							bloc[i].point[6].x=T.x;			bloc[i].point[6].z=16.0f+get_zdec(X,Y+2)+T.z;
							bloc[i].point[7].x=8.0f+T.x;	bloc[i].point[7].z=16.0f+get_zdec(X|1,Y+2)+T.z;
							bloc[i].point[8].x=16.0f+T.x;	bloc[i].point[8].z=16.0f+get_zdec(X+2,Y+2)+T.z;
							bloc[i].point[0].y=get_nh(X,Y);
							bloc[i].point[1].y=get_nh(X|1,Y);
							bloc[i].point[2].y=get_nh(X+2,Y);
							bloc[i].point[3].y=get_nh(X,Y|1);
							bloc[i].point[4].y=get_nh(X|1,Y|1);
							bloc[i].point[5].y=get_nh(X+2,Y|1);
							bloc[i].point[6].y=get_nh(X,Y+2);
							bloc[i].point[7].y=get_nh(X|1,Y+2);
							bloc[i].point[8].y=get_nh(X+2,Y+2);
						}
						else
						{
							bloc[i].point[0].x=T.x;			bloc[i].point[0].z=T.z;
							bloc[i].point[1].x=8.0f+T.x;	bloc[i].point[1].z=T.z;
							bloc[i].point[2].x=16.0f+T.x;	bloc[i].point[2].z=T.z;
							bloc[i].point[3].x=T.x;			bloc[i].point[3].z=8.0f+T.z;
							bloc[i].point[4].x=8.0f+T.x;	bloc[i].point[4].z=8.0f+T.z;
							bloc[i].point[5].x=16.0f+T.x;	bloc[i].point[5].z=8.0f+T.z;
							bloc[i].point[6].x=T.x;			bloc[i].point[6].z=16.0f+T.z;
							bloc[i].point[7].x=8.0f+T.x;	bloc[i].point[7].z=16.0f+T.z;
							bloc[i].point[8].x=16.0f+T.x;	bloc[i].point[8].z=16.0f+T.z;
							bloc[i].point[0].y=get_h(X,Y);
							bloc[i].point[1].y=get_h(X|1,Y);
							bloc[i].point[2].y=get_h(X+2,Y);
							bloc[i].point[3].y=get_h(X,Y|1);
							bloc[i].point[4].y=get_h(X|1,Y|1);
							bloc[i].point[5].y=get_h(X+2,Y|1);
							bloc[i].point[6].y=get_h(X,Y+2);
							bloc[i].point[7].y=get_h(X|1,Y+2);
							bloc[i].point[8].y=get_h(X+2,Y+2);
						}
						map_data(X, Y).setFlat();
						for (int f = 1; f < 9; ++f)			// Check if it's flat
						{
                            if (!Math::Equals(bloc[i].point[0].y, bloc[i].point[f].y))
							{
								map_data(X, Y).unsetFlat();
								break;
							}
						}
					}
				}

                if (bloc[i].tex != old_tex || buf_size >= 500 || ox + 1 < x)
				{
					if (buf_size > 0)
						glDrawRangeElements(GL_TRIANGLE_STRIP, 0, buf_size*9, index_size,GL_UNSIGNED_SHORT,buf_i);		// dessine le tout
					buf_size = 0;
					index_size = 0;
					was_flat = false;
					if (old_tex != bloc[i].tex)
					{
						old_tex = bloc[i].tex;
                        bloc[i].tex->bind();
					}
				}
				ox = x;

				size_t buf_pos = buf_size * 9U;
				if (!FLAT)
				{
					for (int e = 0; e < 9; ++e) // Copie le bloc
						buf_p[buf_pos + e] = bloc[i].point[e];
				}
				else
				{
					for (int e = 0; e < 9; ++e) // Copie le bloc
					{
						buf_p[buf_pos + e].x = flat[e].x + T.x;
						buf_p[buf_pos + e].y = flat[e].y;
						buf_p[buf_pos + e].z = flat[e].z + T.z;
					}
				}

				uint8 *color = buf_c + (buf_pos << 2);
				if (FLAT )
					for(int e = 0 ; e < 36 ; e += 4)
					{
						color[e] = color[e|1] = color[e|2] = 255;
						color[e|3] = 192;
					}
				else
					for(int e = 0 ; e < 36 ; e += 4)
						color[e] = color[e|1] = color[e|2] = color[e|3] = 255;

				bool is_clean = true;
				if (fog_of_war != FOW_DISABLED )
				{
					int Z;
					int grey = 0;
					int black = 0;
					Z = Y + get_zdec_notest(X, Y);							if (Z >= bloc_h_db - 1)	Z = bloc_h_db - 2;
					if (!(view_map(x, Z >> 1) & player_mask))				{	color[0] = color[1] = color[2] = 0;	++black;	}
					else if (!(sight_map(x, Z >> 1) & player_mask))		    {	color[0] = color[1] = color[2] = 127;	++grey;		}
					if (X + 2 < bloc_w_db)
					{
						Z = Y + get_zdec_notest(X + 2, Y);					if (Z >= bloc_h_db - 1)	Z = bloc_h_db - 2;
						if (!(view_map(x + 1, Z >> 1) & player_mask))	    {	color[8] = color[9] = color[10] = 0;		++black;	}
						else if (!(sight_map(x + 1, Z >> 1) & player_mask))	{	color[8] = color[9] = color[10] = 127;	++grey;		}
					}
					if (Y + 2 < bloc_h_db )
					{
						Z = Y + 2 + get_zdec_notest(X, Y + 2);				if (Z >= bloc_h_db - 1)	Z = bloc_h_db - 2;
						if (!(view_map(x, Z >> 1) & player_mask))			{	color[24] = color[25] = color[26] = 0;	++black;	}
						else if (!(sight_map(x, Z >> 1) & player_mask))		{	color[24] = color[25] = color[26] = 127;	++grey;		}
						if (X + 2 < bloc_w_db)
						{
							Z = Y + 2 + get_zdec_notest(X + 2, Y + 2);			if (Z >= bloc_h_db - 1)	Z = bloc_h_db - 2;
							if (!(view_map(x + 1, Z >> 1) & player_mask))		{	color[32] = color[33] = color[34] = 0;	++black;	}
							else if (!(sight_map(x + 1, Z >> 1) & player_mask))	{	color[32] = color[33] = color[34] = 127;	++grey;		}
						}
					}
					is_clean = grey == 4 || black == 4 || ( grey == 0 && black == 0 );
					if (!FLAT && !map_data(X, Y).isFlat() && !lp_CONFIG->low_definition_map)
					{
						color[4]  = color[5]  = color[6]  = uint8((color[0] + color[8]) >> 1);
						color[12] = color[13] = color[14] = uint8((color[0] + color[24]) >> 1);
						color[20] = color[21] = color[22] = uint8((color[8] + color[32]) >> 1);
						color[16] = color[17] = color[18] = uint8((color[12] + color[20]) >> 1);
						color[28] = color[29] = color[30] = uint8((color[24] + color[32]) >> 1);
					}
				}

#if !defined DEBUG_UNIT_POS && !defined DEBUG_ENERGY
				if (FLAT || map_data(X, Y).isFlat() || lp_CONFIG->low_definition_map )
				{
					if (was_flat && bloc[i].tex_x == bloc[ bmap(x - 1, y) ].tex_x + 1 && is_clean && was_clean && (FLAT || map_data(X, Y).isFlat()) )
					{
						buf_i[ index_size-4 ] = GLushort(2 + buf_pos);
						buf_i[ index_size-2 ] = GLushort(8 + buf_pos);
						buf_i[ index_size-1 ] = GLushort(2 + buf_pos);
					}
					else
					{
						buf_i[ index_size++ ] = GLushort(buf_pos);
						buf_i[ index_size++ ] = GLushort(2 + buf_pos);
						buf_i[ index_size++ ] = GLushort(6 + buf_pos);
						buf_i[ index_size++ ] = GLushort(8 + buf_pos);
						buf_i[ index_size++ ] = GLushort(2 + buf_pos);
						was_flat = FLAT || map_data(X, Y).isFlat();     // If it's only lp_CONFIG->low_definition_map, it cannot be considered flat
					}
				}
				else
				{
#endif
					was_flat = false;
					buf_i[ index_size++ ] = GLushort(buf_pos);
					buf_i[ index_size++ ] = GLushort(1 + buf_pos);
					buf_i[ index_size++ ] = GLushort(3 + buf_pos);
					buf_i[ index_size++ ] = GLushort(4 + buf_pos);
					buf_i[ index_size++ ] = GLushort(6 + buf_pos);
					buf_i[ index_size++ ] = GLushort(7 + buf_pos);
					buf_i[ index_size++ ] = GLushort(7 + buf_pos);
					buf_i[ index_size++ ] = GLushort(8 + buf_pos);
					buf_i[ index_size++ ] = GLushort(4 + buf_pos);
					buf_i[ index_size++ ] = GLushort(5 + buf_pos);
					buf_i[ index_size++ ] = GLushort(1 + buf_pos);
					buf_i[ index_size++ ] = GLushort(2 + buf_pos);
#if !defined DEBUG_UNIT_POS && !defined DEBUG_ENERGY
				}
#endif
				was_clean = is_clean;
				T.x -= float(x << 4);
				memcpy(buf_t + (buf_pos << 1), bloc[i].texcoord, 72);		// texture

#ifdef DEBUG_UNIT_POS
				int Z;
				Z = Y + get_zdec_notest(X, Y);					if (Z >= bloc_h_db - 1)	Z = bloc_h_db - 2;
				Z &= 0xFFFFFE;
				X &= 0xFFFFFE;
				if (map_data(X,Z).unit_idx != -1)		// Shows unit's pos on map
				{
					color[0]=color[1]=color[2]=color[3]=color[4]=color[5]=color[6]=color[7]=color[12]=color[13]=color[14]=color[15]=color[16]=color[17]=color[18]=color[19]=0;
					if (map_data(X,Z).unit_idx>=0 )		// Shows unit's pos on map
						color[0]=color[4]=color[12]=color[16]=255;
					else		// It's a feature
						color[1]=color[5]=color[13]=color[17]=255;
				}
				else if (!map_data(X,Z).air_idx.empty() )		// Shows unit's pos on map
				{
					color[0]=color[1]=color[2]=color[3]=color[4]=color[5]=color[6]=color[7]=color[12]=color[13]=color[14]=color[15]=color[16]=color[17]=color[18]=color[19]=0;
					color[2]=color[6]=color[14]=color[18]=255;
				}
				if (map_data(X+1,Z).unit_idx!=-1)		// Shows unit's pos on map
				{
					color[8]=color[9]=color[10]=color[11]=color[20]=color[21]=color[22]=color[23]=0;
					if (map_data(X+1,Z).unit_idx>=0 )		// Shows unit's pos on map
						color[8]=color[20]=255;
					else
						color[9]=color[21]=255;
				}
				else if (!map_data(X+1,Z).air_idx.empty() )		// Shows unit's pos on map
				{
					color[8]=color[9]=color[10]=color[11]=color[20]=color[21]=color[22]=color[23]=0;
					color[10]=color[22]=255;
				}
				if (map_data(X,Z+1).unit_idx!=-1 )		// Shows unit's pos on map
				{
					color[24]=color[25]=color[26]=color[27]=color[28]=color[29]=color[30]=color[31]=0;
					if (map_data(X,Z+1).unit_idx>=0 )		// Shows unit's pos on map
						color[24]=color[28]=255;
					else
						color[25]=color[29]=255;
				}
				else if (!map_data(X,Z+1).air_idx.empty() )		// Shows unit's pos on map
				{
					color[24]=color[25]=color[26]=color[27]=color[28]=color[29]=color[30]=color[31]=0;
					color[26]=color[30]=255;
				}
				if (map_data(X+1,Z+1).unit_idx!=-1 )		// Shows unit's pos on map
				{
					color[32]=color[33]=color[34]=color[35]=0;
					if (map_data(X+1,Z+1).unit_idx>=0 )		// Shows unit's pos on map
						color[32]=255;
					else
						color[33]=255;
				}
				else if (!map_data(X+1,Z+1).air_idx.empty() )		// Shows unit's pos on map
				{
					color[32]=color[33]=color[34]=color[35]=0;
					color[34]=255;
				}
#elif defined DEBUG_RADAR_MAP
				int Z;
				Z=Y+get_zdec_notest(X,Y);					if (Z>=bloc_h_db-1)	Z=bloc_h_db-2;
				Z&=0xFFFFFE;
				X&=0xFFFFFE;
				if ((radar_map->line[Z>>1][X>>1] & player_mask) )		// Shows unit's pos on map
					for(i=0;i<9;i++)
					{
						color[i<<2]=color[(i<<2)+1]=color[(i<<2)+2]=color[(i<<2)+3]=0;
						color[(i<<2)]=255;
					}
				else
				{
					if ((sonar_map->line[Z >> 1][X >> 1] & player_mask)) // Shows unit's pos on map
					{
						for (i = 0; i < 9; ++i)
						{
							color[i << 2] = color[(i << 2) + 1] = color[(i << 2) + 2] = color[(i << 2) + 3] = 0;
							color[(i << 2) + 2] = 255;
						}
					}
				}
#elif defined DEBUG_ENERGY
				int Z;
				Z=Y+get_zdec_notest(X,Y);					if (Z>=bloc_h_db-1)	Z=bloc_h_db-2;
				Z&=0xFFFFFE;
				X&=0xFFFFFE;
				for(i = 0 ; i < 9 ; ++i)
					color[i<<2] = color[(i<<2)+1] = color[(i<<2)+2] = color[(i<<2)+3] = Math::Max(Math::Max(energy(X,Z), energy(X+1,Z)),
																								  Math::Max(energy(X,Z+1), energy(X+1,Z+1))) * IEmax;
#endif
				++buf_size;
			}
			if (buf_size > 0)
			{
				glDrawRangeElements(GL_TRIANGLE_STRIP, 0, buf_size*9, index_size,GL_UNSIGNED_SHORT,buf_i);		// dessine le tout
				was_flat = false;
				index_size=0;
				buf_size=0;
			}
		}
		glDisableClientState(GL_COLOR_ARRAY);		// Couleurs(pour le brouillard de guerre)

        detail_shader->release();

		gfx->unlock();

		glActiveTextureARB(GL_TEXTURE1_ARB);
		glClientActiveTextureARB(GL_TEXTURE1_ARB);
		glDisable(GL_TEXTURE_2D);

		glActiveTextureARB(GL_TEXTURE0_ARB);
		glEnable(GL_TEXTURE_2D);
		glClientActiveTextureARB(GL_TEXTURE0_ARB);

		glPopMatrix();
	}

	Vector3D MAP::hit(Vector3D Pos, Vector3D Dir, bool water, float length, bool allow_out) const			// Calcule l'intersection d'un rayon avec la carte(le rayon partant du dessus de la carte)
	{
        if (Math::Zero(Dir.x) && Math::Zero(Dir.z)) // Solution triviale
		{
			Pos.y = get_unit_h(Pos.x, Pos.z);
			return Pos;
		}

		if (get_unit_h(Pos.x,Pos.z) > Pos.y)		// Cas non traité
			return Pos;

		float step = 1.0f;
        if (!Math::Zero(Dir.x) && !Math::Zero(Dir.z))
		{
			if (fabsf(Dir.x) < fabsf(Dir.z))
				step = 1.0f / fabsf(Dir.x);
			else
				step = 1.0f / fabsf(Dir.z);
		}
		int nb = 0;
		int nb_limit = (int)(Pos.y) + 1000;
		const float dwm = (float)map_w_d;
		const float dhm = (float)map_h_d;
		Dir = (1.0f * step) * Dir;
		float len_step = Dir.norm();
		while (((sealvl < Pos.y && water) || !water) && get_max_h((int)(Pos.x + (float)map_w_d) >> 3,(int)(Pos.z + (float)map_h_d) >> 3) < Pos.y)
		{
			if (nb >= nb_limit || length < 0.0f)
				return Pos;
			length -= len_step;
			nb++;
			Pos += Dir;
			if (!allow_out && (fabsf(Pos.x) > dwm || fabsf(Pos.z) > dhm)) // Pas de résultat
				return Pos;
		}
		length += len_step;
		Pos -= Dir;

		while (((sealvl < Pos.y && water) || !water) && get_unit_h(Pos.x, Pos.z) < Pos.y)
		{
			if (nb >= nb_limit || length < 0.0f)
				return Pos;
			length -= len_step;
			++nb;
			Pos += Dir;
			if (!allow_out && (fabsf(Pos.x) > dwm || fabsf(Pos.z) > dhm)) // Pas de résultat
				return Pos;
		}

		for (int i = 0; i < 7; ++i)
		{
			length += len_step;
			Pos -= Dir; 	// On recommence la dernière opération mais avec plus de précision
			Dir *= 0.5f;
			len_step *= 0.5f;
			nb = 0;
			while (((sealvl < Pos.y && water) || !water) && get_unit_h(Pos.x, Pos.z) < Pos.y)
			{
				if (nb >= 2 || length < 0.0f)
					return Pos;
				length -= len_step;
				++nb;
				Pos += Dir;
			}
		}
		return Pos;		// Meilleure solution approximative trouvée
	}



	int MAP::check_metal(int x1, int y1, int unit_idx, int *stuff_id) const
	{
		if (unit_idx < 0 || unit_idx >= unit_manager.nb_unit)
			return 0;

		const int w = unit_manager.unit_type[ unit_idx ]->FootprintX;
		const int h = unit_manager.unit_type[ unit_idx ]->FootprintZ;
		int metal_base = 0;
		const int end_y = y1 + (h >> 1);
		const int end_x = x1 + (w >> 1);
		const int start_x = x1 - (w >> 1);
		for (int ry = y1 - (h >> 1); ry <= end_y ; ++ry)
		{
			if (ry >= 0 && ry < bloc_h_db)
			{
				for (int rx = start_x ; rx <= end_x ; ++rx)
				{
					if (rx >= 0 && rx < bloc_w_db)
					{
						if (map_data(rx, ry).stuff >=0)
						{
                            features.lock();
							const int ID = map_data(rx, ry).stuff;
							if (ID >= 0)            // We have to recheck this in case it has changed before locking
                            {
								const int type = features.feature[ ID ].type;
								const Feature* const feature = feature_manager.getFeaturePointer(type);
                                if (feature && !feature->reclaimable && !feature->blocking)
                                {
                                    metal_base += feature->metal;
                                    if (stuff_id)           // We need to know where to put metal extractors, so it'll give the impression the AI is clever :P
										*stuff_id = ID;
                                }
                            }
                            features.unlock();
                        }
					}
				}
			}
		}
		if (metal_base == 0)
			metal_base = ota_data.SurfaceMetal;
		return metal_base;
	}

	void MAP::drawCircleOnMap(const float x, const float y, const float radius, const uint32 color, const float thickness) const
	{
		const int steps = std::max(5, int(radius * 2.0f));
		const float f = 2.0f * (float)M_PI / (float)steps;
		std::vector<Vector3D> vertices0, vertices1;
		std::vector<uint32> colors0, colors1;
		vertices0.reserve(steps * 2 + 2);
		vertices1.reserve(steps * 2 + 2);
		colors0.reserve(steps * 2 + 2);
		colors1.reserve(steps * 2 + 2);
		const uint32 mask = makeacol(0xFF,0xFF,0xFF,0x0);
		for(int i = 0 ; i <= steps ; ++i)
		{
			const float _cos = std::cos((float)i * f);
			const float _sin = std::sin((float)i * f);
			const float tx = x + (radius + thickness) * _cos;
			const float tz = y + (radius + thickness) * _sin;
			const float ty = get_unit_h(tx, tz) + 1.0f;
			const float mx = x + radius * _cos;
			const float mz = y + radius * _sin;
			const float my = get_unit_h(mx, mz) + 1.0f;
			const float bx = x + (radius - thickness) * _cos;
			const float bz = y + (radius - thickness) * _sin;
			const float by = get_unit_h(bx, bz) + 1.0f;
			const Vector3D T(tx, ty, tz);
			const Vector3D M(mx, my, mz);
			const Vector3D B(bx, by, bz);

			vertices0.push_back(T);		colors0.push_back(color & mask);
			vertices0.push_back(M);		colors0.push_back(color);

			vertices1.push_back(M);		colors1.push_back(color);
			vertices1.push_back(B);		colors1.push_back(color & mask);
		}
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);
		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);

		glVertexPointer(3, GL_FLOAT, 0, &(vertices0.front()));
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, &(colors0.front()));
		glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei)vertices0.size());

		glVertexPointer(3, GL_FLOAT, 0, &(vertices1.front()));
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, &(colors1.front()));
		glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei)vertices1.size());

		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
	}

	void WATER::draw(float t, bool shaded)
	{
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glTranslatef(cosf(t),0.0f,sinf(t));
		if (shaded)
		{
			glBegin(GL_QUADS);
			glTexCoord3f(-map_w/w+0.5f,-map_h/w+0.5f,1.0f);		glVertex3f(-map_w,0.0f,-map_h);
			glTexCoord3f(map_w/w+0.5f,-map_h/w+0.5f,1.0f);		glVertex3f(map_w,0.0f,-map_h);
			glTexCoord3f(map_w/w+0.5f,map_h/w+0.5f,1.0f);		glVertex3f(map_w,0.0f,map_h);
			glTexCoord3f(-map_w/w+0.5f,map_h/w+0.5f,1.0f);		glVertex3f(-map_w,0.0f,map_h);

			glTexCoord2f(-1.0f,-1.0f);

			glTexCoord3f(-map_w/w+0.5f,-map_h/w+0.5f,1.0f);		glVertex4f(-map_w,0.0f,-map_h,1.0f);
			glTexCoord3f(map_w/w+0.5f,-map_h/w+0.5f,1.0f);		glVertex4f(map_w,0.0f,-map_h,1.0f);
			glTexCoord3f(map_w/w+0.5f,-map_h/w+0.5f,0.0f);		glVertex4f(map_w,0.0f,-map_h,0.0f);
			glTexCoord3f(-map_w/w+0.5f,-map_h/w+0.5f,0.0f);		glVertex4f(-map_w,0.0f,-map_h,0.0f);

			glTexCoord3f(map_w/w+0.5f,-map_h/w+0.5f,1.0f);		glVertex4f(map_w,0.0f,-map_h,1.0f);
			glTexCoord3f(map_w/w+0.5f,map_h/w+0.5f,1.0f);		glVertex4f(map_w,0.0f,map_h,1.0f);
			glTexCoord3f(map_w/w+0.5f,map_h/w+0.5f,0.0f);		glVertex4f(map_w,0.0f,map_h,0.0f);
			glTexCoord3f(map_w/w+0.5f,-map_h/w+0.5f,0.0f);		glVertex4f(map_w,0.0f,-map_h,0.0f);

			glTexCoord3f(map_w/w+0.5f,map_h/w+0.5f,1.0f);		glVertex4f(map_w,0.0f,map_h,1.0f);
			glTexCoord3f(-map_w/w+0.5f,map_h/w+0.5f,1.0f);		glVertex4f(-map_w,0.0f,map_h,1.0f);
			glTexCoord3f(-map_w/w+0.5f,map_h/w+0.5f,0.0f);		glVertex4f(-map_w,0.0f,map_h,0.0f);
			glTexCoord3f(map_w/w+0.5f,map_h/w+0.5f,0.0f);		glVertex4f(map_w,0.0f,map_h,0.0f);

			glTexCoord3f(-map_w/w+0.5f,map_h/w+0.5f,1.0f);		glVertex4f(-map_w,0.0f,map_h,1.0f);
			glTexCoord3f(-map_w/w+0.5f,-map_h/w+0.5f,1.0f);		glVertex4f(-map_w,0.0f,-map_h,1.0f);
			glTexCoord3f(-map_w/w+0.5f,-map_h/w+0.5f,0.0f);		glVertex4f(-map_w,0.0f,-map_h,0.0f);
			glTexCoord3f(-map_w/w+0.5f,map_h/w+0.5f,0.0f);		glVertex4f(-map_w,0.0f,map_h,0.0f);
			glEnd();
			return;
		}
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f,0.0f);		glVertex3f(-map_w*0.5f,0.0f,-map_h*0.5f);
		glTexCoord2f(1.0f,0.0f);		glVertex3f(map_w*0.5f,0.0f,-map_h*0.5f);
		glTexCoord2f(1.0f,1.0f);		glVertex3f(map_w*0.5f,0.0f,map_h*0.5f);
		glTexCoord2f(0.0f,1.0f);		glVertex3f(-map_w*0.5f,0.0f,map_h*0.5f);
		glEnd();

		glDisable( GL_TEXTURE_2D );
		glColor4ub(0,0,0,0xFF);

		glBegin(GL_QUADS);
		glVertex3f(-map_w,0.0f,-map_h);
		glVertex3f(map_w,0.0f,-map_h);
		glVertex3f(map_w,0.0f,-map_h*0.5f);
		glVertex3f(-map_w,0.0f,-map_h*0.5f);

		glVertex3f(-map_w,0.0f,map_h*0.5f);
		glVertex3f(map_w,0.0f,map_h*0.5f);
		glVertex3f(map_w,0.0f,map_h);
		glVertex3f(-map_w,0.0f,map_h);

		glVertex3f(-map_w,0.0f,-map_h*0.5f);
		glVertex3f(-map_w*0.5f,0.0f,-map_h*0.5f);
		glVertex3f(-map_w*0.5f,0.0f,map_h*0.5f);
		glVertex3f(-map_w,0.0f,map_h*0.5f);

		glVertex3f(map_w*0.5f,0.0f,-map_h*0.5f);
		glVertex3f(map_w,0.0f,-map_h*0.5f);
		glVertex3f(map_w,0.0f,map_h*0.5f);
		glVertex3f(map_w*0.5f,0.0f,map_h*0.5f);
		glEnd();
	}

} // namespace TA3D

