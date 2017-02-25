/*  TA3D, a remake of Total Annihilation
	Copyright (C) 2005  Roland BROCHARD

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

/*-----------------------------------------------------------------------------------\
  |                                         tnt.cp                                     |
  |  ce fichier contient les structures, classes et fonctions nécessaires à la lecture |
  | des fichiers tnt de total annihilation qui sont les fichiers contenant les cartes  |
  | du jeu.                                                                            |
  |                                                                                    |
  \-----------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "misc/matrix.h"
#include "TA3D_NameSpace.h"
#include "ta3dbase.h"
#include "EngineClass.h"
#include "tdf.h"
#include "tnt.h"
#include "misc/math.h"
#include "logs/logs.h"


#define READ(X) file->read((char*)&(X), sizeof(X))

namespace TA3D
{

	namespace
	{

        QImage load_tnt_minimap_bmp(TNTMINIMAP *minimap,int *sw,int *sh)
		{
            // Copy the mini-map into an 8-bit QImage
            QImage mini = gfx->create_surface_ex(8,TNTMINIMAP_WIDTH,TNTMINIMAP_HEIGHT);
			for(int y = 0; y < TNTMINIMAP_HEIGHT; ++y)
                memcpy((char*)mini.scanLine(y), minimap->map[y],TNTMINIMAP_WIDTH);

			// Apply the palette -- increase the color depth
            convert_format(mini);

			// Examine the image for a blank-looking bottom or right edge
			int mini_w = TNTMINIMAP_WIDTH;
			int mini_h = TNTMINIMAP_HEIGHT;
			uint32 blank_color = makecol(120,148,252); // approximately
			uint32 mask = makecol(0xFC, 0xFC, 0xFC); // XXX this assumes 24- or 32-bit pixels
			do
			{
				--mini_w;
			} while ( mini_w > 0 &&
					( ( SurfaceInt(mini, mini_w, 0) & mask) == blank_color ||
                        SurfaceInt(mini, mini_w, 0) == 0));
			do
			{
				--mini_h;
			} while( mini_h > 0 &&
				( ( SurfaceInt(mini, 0, mini_h) & mask) == blank_color ||
                SurfaceInt(mini,0,mini_h) == 0));
			mini_w++;
			mini_h++;

			if (sw) *sw = mini_w;
			if (sh) *sh = mini_h;

			return mini;
		}


        QImage load_tnt_minimap_fast_raw_bmp(const QString& filename, int& sw, int& sh)
		{
            QIODevice *headerBytes = VFS::Instance()->readFileRange(filename, 0, sizeof(TNTHEADER));
			if (headerBytes == NULL)
                return QImage();

            const QByteArray &buffer = headerBytes->read(sizeof(TNTHEADER));
            const TNTHEADER *header = &((const TNTHEADER_U*)buffer.data())->header;

            QIODevice *minimapdata = VFS::Instance()->readFileRange(filename, header->PTRminimap, sizeof(TNTMINIMAP));
			if (!minimapdata)
			{
				delete headerBytes;
                return QImage();
			}

			minimapdata->seek(header->PTRminimap);
			TNTMINIMAP minimap;
            minimapdata->read((char*)&minimap, sizeof(minimap));
            QImage bitmap = load_tnt_minimap_bmp(&minimap, &sw, &sh);

			delete headerBytes;
			delete minimapdata;

			return bitmap;
		}


	} // unnamed namespace






    MAP	*load_tnt_map(QIODevice *file)		// Charge une map au format TA, extraite d'une archive HPI/UFO
	{
		LOG_DEBUG("MAP: creating MAP object ...");
		MAP	*map = new MAP;		// Crée une nouvelle carte

		the_map = map;

		LOG_DEBUG("MAP: initializing map data ...");

		map->init();
		map->tnt = true;
		TNTHEADER_U	header_u;		// Structure pour l'en-tête du fichier

		LOG_DEBUG("MAP: reading header");

        READ(header_u);
		TNTHEADER	&header = header_u.header;		// Structure pour l'en-tête du fichier

		# ifdef TNT_DEBUG_MODE
		LOG_DEBUG("[tnt - load map] IDversion = " << header.IDversion);
		LOG_DEBUG("[tnt - load map] Width = " << header.Width);
		LOG_DEBUG("[tnt - load map] Height = ",header.Height);
		LOG_DEBUG("[tnt - load map] tiles = ",header.tiles);
		LOG_DEBUG("[tnt - load map] tileanims = " << header.tileanims);
		LOG_DEBUG("[tnt - load map] sealevel = " << header.sealevel);
		# endif

		LOG_DEBUG("MAP: reading TDF table");
		int *TDF_index = new int[header.tileanims];

		int i, x, y;
		for (i = 0; i < header.tileanims; ++i) // Crée le tableau pour la correspondance des éléments
		{
			file->seek(header.PTRtileanim + 4 + (i * 132));
            const QString &fname = QString::fromLatin1(getString(file));
			TDF_index[i] = feature_manager.get_feature_index(fname);
			if (TDF_index[i] == -1)
				LOG_ERROR("tdf not found: " << fname);
		}

		map->sealvl = float(header.sealevel) * H_DIV;
		// Lit la minimap
		LOG_DEBUG("MAP: reading mini map");
		int event_timer = msec_timer;
		int w,h;
		file->seek(header.PTRminimap);
        READ(w);
        READ(h);
		map->mini_w = w;
		map->mini_h = h;
		map->mini = gfx->create_surface_ex(8, 252, 252);
		for(y = 0; y < 252; ++y)
            file->read((char*)map->mini.scanLine(y), 252);
        convert_format(map->mini);
		map->mini_w = 251;
		map->mini_h = 251;
		uint32 mask = makecol(0xFC, 0xFC, 0xFC);
		while (map->mini_w > 0 && ( ( SurfaceInt(map->mini,map->mini_w,0) & mask) == makecol(120,148,252) || SurfaceInt(map->mini,map->mini_w,0) == 0))
			--(map->mini_w);
		while (map->mini_h > 0 && ( ( SurfaceInt(map->mini,0,map->mini_h) & mask) == makecol(120,148,252) || SurfaceInt(map->mini,0,map->mini_h) == 0))
			--(map->mini_h);
		++(map->mini_w);
		++(map->mini_h);
		if (g_useTextureCompression && lp_CONFIG->use_texture_compression)
			gfx->set_texture_format(GL_COMPRESSED_RGB_ARB);
		else
			gfx->set_texture_format(gfx->defaultTextureFormat_RGB());
		map->glmini = gfx->make_texture(map->mini,FILTER_LINEAR,true);

		LOG_INFO("minimap read in " << float(msec_timer - event_timer) * 0.001f << "s.");

		// Lit les différents morceaux
		LOG_DEBUG("MAP: reading blocs data");
		event_timer = msec_timer;
        int n_bmp = (header.tiles+0x3F) >> 5; // How many 1024x32 textures are required to store all chunks
        QImage *bmp_tex = new QImage[n_bmp];
		for (i = 0; i < n_bmp; ++i)
			bmp_tex[i] = gfx->create_surface_ex(8, 1024, 32);

		file->seek(header.PTRtilegfx);
        for (i = 0; i < header.tiles; ++i) // Read all the chunks
		{
            int tex_num = i>>5;	// Associated texture ID
            int tx = (i&0x1F)<<5;			// Texture coordinates
            for(y = 0; y < 32; ++y)	// Read the chunk
                file->read((char*)bmp_tex[tex_num].scanLine(y) + tx, 32);
		}

		LOG_DEBUG("MAP: allocating map memory");
		map->bloc_w       = header.Width >> 1;
		map->bloc_h       = header.Height >> 1;
		map->bloc_w_db    = map->bloc_w << 1;
		map->bloc_h_db    = map->bloc_h << 1;
		map->map_h        = map->bloc_h << 4;
		map->map_w        = map->bloc_w << 4;
		map->map_h_d      = map->bloc_h << 3;
		map->map_w_d      = map->bloc_w << 3;
		map->map2blocdb_w = float(map->bloc_w_db) / float(map->map_w);
		map->map2blocdb_h = float(map->bloc_h_db) / float(map->map_h);

		map->bmap.resize(map->bloc_w, map->bloc_h);
		map->view.resize(map->bloc_w, map->bloc_h);
		map->map_data.resize(map->bloc_w_db, map->bloc_h_db);
		map->path.resize(map->bloc_w_db, map->bloc_h_db);
		map->energy.resize(map->bloc_w_db, map->bloc_h_db);
		map->slope.resize(map->bloc_w_db, map->bloc_h_db);
		map->obstacles.resize(map->bloc_w_db, map->bloc_h_db);
		map->obstacles.clear();

		LOG_DEBUG("MAP: creating FOW maps");
		map->sight_map.resize(map->bloc_w, map->bloc_h);		// FOW maps
		map->view_map.resize(map->bloc_w, map->bloc_h);
		map->radar_map.resize(map->bloc_w, map->bloc_h);
		map->sonar_map.resize(map->bloc_w, map->bloc_h);
		map->view_map.clear(0);
		map->sight_map.clear(0);
		map->radar_map.clear(0);
		map->sonar_map.clear(0);

		LOG_DEBUG("MAP: allocating height maps");
		map->h_map.resize(map->bloc_w_db, map->bloc_h_db);
		map->ph_map.resize(map->bloc_w_db, map->bloc_h_db);

		LOG_DEBUG("MAP: initialising map data");
		map->path.clear();

		map->view.clear(0);
		map->nbbloc = header.tiles;		// Nombre de blocs nécessaires
		map->bloc = new BLOC[map->nbbloc];	// Alloue la mémoire pour les blocs
		map->ntex = short(n_bmp);
		map->tex = new GLuint[n_bmp];	// Tableau d'indices de texture OpenGl

		for (i = 0 ; i < map->nbbloc ; i++) // Crée les blocs
		{
			map->bloc[i].init();
			const int tex_num = i >> 5;	// Numéro de la texture associée
			const int tx = (i & 0x1F) << 5;			// Coordonnées sur la texture
			int r = 0, g = 0, b = 0;
			for (y = 0; y < 32; ++y)
			{
				for (x = tx; x < tx + 32; ++x)
				{
					const int c = SurfaceByte(bmp_tex[tex_num],x,y);
                    const QRgb &qc = pal.at(c);
                    r += qRed(qc);
                    g += qGreen(qc);
                    b += qBlue(qc);
				}
			}
			r >>= 10;
			g >>= 10;
			b >>= 10;
			map->bloc[i].lava = (r > 4 && g < (r >> 2) && b < (r >> 2));
			map->bloc[i].tex_x = byte(tx >> 5);
		}

		LOG_INFO("Blocs read in " << float(msec_timer-event_timer) * 0.001f << "s.");
		event_timer = msec_timer;

		LOG_DEBUG("MAP: creating textures");

		if (g_useTextureCompression && lp_CONFIG->use_texture_compression)
			gfx->set_texture_format(GL_COMPRESSED_RGB_ARB);
		else
			gfx->set_texture_format(gfx->defaultTextureFormat_RGB());
        for (i = 0; i < n_bmp; ++i) // Finish loading textures
		{
            QImage tmp = convert_format_24_copy(bmp_tex[i]);
			map->tex[i] = gfx->make_texture(tmp);
		}
		LOG_INFO("Textures for blocks in " << float(msec_timer - event_timer) * 0.001f << "s.");

		event_timer = msec_timer;

		map->lvl = new Vector3D*[map->bloc_w*map->bloc_h];
		for (i = 0; i < map->bloc_w * map->bloc_h; ++i)
			map->lvl[i] = NULL;

		LOG_DEBUG("MAP: creating blocs texture coordinates");
		for(i = 0; i < map->nbbloc; ++i) // Crée les blocs
		{
			// Numéro de texture
			int t_n = i >> 5;
			// Position sur la texture
			float t_x = ((float) (i&0x1F)) / 32.f;

			map->bloc[i].tex      = map->tex[t_n];
			map->bloc[i].nbpoint  = 9;
			map->bloc[i].nbindex  = 12;
			map->bloc[i].texcoord = new float[map->bloc[i].nbpoint<<1];

			const float c = (1.f / 32.f) - (1.f / 1024.f);
			t_x += 1.f / 2048.f;
			map->bloc[i].texcoord[0]  = t_x;              map->bloc[i].texcoord[1]  = 1.0f / 64.0f;
			map->bloc[i].texcoord[2]  = t_x + c * 0.5f;   map->bloc[i].texcoord[3]  = 1.0f / 64.0f;
			map->bloc[i].texcoord[4]  = t_x + c;          map->bloc[i].texcoord[5]  = 1.0f / 64.0f;
			map->bloc[i].texcoord[6]  = t_x;              map->bloc[i].texcoord[7]  = 0.5f;
			map->bloc[i].texcoord[8]  = t_x + c * 0.5f;   map->bloc[i].texcoord[9]  = 0.5f;
			map->bloc[i].texcoord[10] = t_x + c;          map->bloc[i].texcoord[11] = 0.5f;
			map->bloc[i].texcoord[12] = t_x;              map->bloc[i].texcoord[13] = 63.0f / 64.0f;
			map->bloc[i].texcoord[14] = t_x + c * 0.5f;   map->bloc[i].texcoord[15] = 63.0f / 64.0f;
			map->bloc[i].texcoord[16] = t_x +c;           map->bloc[i].texcoord[17] = 63.0f / 64.0f;
		}

		// Charge les données sur la position des blocs
		int max_tex_size;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE,&max_tex_size);
        max_tex_size = Math::Min(max_tex_size, 2048);           // This texture can be very big, so let's reduce it

		/*---------- code to convert the map to new format -------------------*/
		/*	map->macro_w = map->bloc_w+15>>4;
			map->macro_h = map->bloc_h+15>>4;
			map->macro_bloc = new BLOC*[map->macro_h];
			map->macro_bloc[0] = new BLOC[map->macro_h*macro_w];

			for(i=1;i<map->macro_h;i++)
			map->macro_bloc[i] = &(map->macro_bloc[0][macro_w*i]);

            QImage tmp = gfx->create_surface_ex(32,512,512);
			for(uint32 y=0;y<map->macro_h;y++)
			for(uint32 x=0;x<map->macro_w;x++) {
			for(uint32 py=0;py<16 && (y<<4)+py<map->bloc_h;py++)				// Create texture
			for(uint32 px=0;px<16 && (x<<4)+px<map->bloc_w;px++) {
			i = map->bmap[(y<<4)+py][(x<<4)+px];
			uint32 tex_num = i>>6;
			int tx = (i&0x7)<<5;
			int ty = ((i>>3)&0x7)<<5;
			blit(bmp_tex[tex_num],tmp,tx,ty,px<<4,py<<4,32,32);
			}
			map->macro_bloc[y][x].tex = gfx->make_texture(tmp);
			glBindTexture(GL_TEXTURE_2D,map->macro_bloc[y][x].tex);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
			}
			SDL_FreeSurface(tmp);*/

		/*--------------------------------------------------------------------*/

		LOG_DEBUG("MAP: creating low definition texture and lava map");

        QImage low_def = gfx->create_surface_ex(8, Math::Min(max_tex_size,map->map_w), Math::Min(max_tex_size,map->map_h));
        low_def.fill(0x0);
		file->seek(header.PTRmapdata);
        file->read((char*)(map->bmap.getData()), map->bmap.getSize());
		for (y = 0; y < map->bloc_h; ++y)
		{
			for (x = 0; x < map->bloc_w; ++x)
			{
				if (map->bmap(x, y) >= map->nbbloc)			// To add some security
					map->bmap(x, y) = 0;

				/*---------- code to build the low def map (mega zoom) ---------------*/
				i = map->bmap(x, y);
				const int tex_num = i >> 5;	// Numéro de la texture associée
				const int tx = (i & 0x1F) << 5;			// Coordonnées sur la texture

				stretch_blit(bmp_tex[tex_num], low_def, tx, 0, 32, 32,
                    x * (low_def.width() - 1) / map->bloc_w, y * (low_def.height() - 1) / map->bloc_h,
                    (x + 1) * (low_def.width() - 1) / map->bloc_w - x * (low_def.width() - 1) / map->bloc_w,
                    (y + 1) * (low_def.height() - 1) / map->bloc_h - y * (low_def.height() - 1) / map->bloc_h);
				/*--------------------------------------------------------------------*/
			}
		}
		LOG_INFO("Low definition map image built in " << float(msec_timer - event_timer) * 0.001f << "s.");
		event_timer = msec_timer;

        convert_format_24(low_def);
		gfx->set_texture_format(GL_RGB5);
		map->low_tex = gfx->make_texture(low_def);		// Build the low details texture map

		LOG_INFO("Low definition texture uploaded in " << float(msec_timer - event_timer) * 0.001f << "s.");
		event_timer = msec_timer;

        QImage lava_map = gfx->create_surface_ex(8, Math::Min(map->bloc_w,1024), Math::Min(map->bloc_h,1024));
        lava_map.fill(0x0);
		for (y = 0; y < map->bloc_h; ++y)               // Build the lava map
			for (x = 0; x < map->bloc_w; ++x)
				if (map->bloc[map->bmap(x, y)].lava)
                    circlefill(lava_map, x * lava_map.width() / map->bloc_w, y * lava_map.height() / map->bloc_h, 3, 0xFF);
		LOG_INFO("Lava image built in " << float(msec_timer - event_timer) * 0.001f << "s.");
		event_timer = msec_timer;

		map->lava_map = gfx->make_texture(lava_map,FILTER_LINEAR,true);		// Build the lava texture map

		LOG_INFO("Lava texture uploaded in " << float(msec_timer - event_timer) * 0.001f << "s.");
		event_timer = msec_timer;

		if (g_useTextureCompression && lp_CONFIG->use_texture_compression)
			gfx->set_texture_format(GL_COMPRESSED_RGB_ARB);
		else
			gfx->set_texture_format(gfx->defaultTextureFormat_RGB());

		LOG_DEBUG("MAP: computing height data (step 1)");
		// Charge d'autres données sur les blocs
		map->water = false;
		file->seek(header.PTRmapattr);
		for (y = 0; y< (map->bloc_h << 1); ++y)
		{
			for (x = 0; x < (map->bloc_w << 1);  ++x)
			{
				const int c = byte(readChar(file));
				if (c < header.sealevel)
					map->water = true;
				map->h_map(x, y) = map->ph_map(x, y) = float(c) * H_DIV;
				readChar(file);
				readChar(file);
				readChar(file);
			}
		}

		LOG_DEBUG("MAP: computing height data (step 2)");
        parallel_for<int>(0, map->bloc_h_db, [&](const int y) // Compute additionnal map informations
		{
			for (int x = 0 ; x < map->bloc_w_db ; ++x)
			{
				map->map_data(x, y).init();
				map->map_data(x, y).setUnderwater(map->h_map(x, y) < map->sealvl);
				map->map_data(x, y).setLava(map->bmap(x >> 1, y >> 1) < map->nbbloc ? map->bloc[ map->bmap(x >> 1, y >> 1) ].lava : false);
				if (!map->map_data(x, y).isLava() && (x>>1) + 1 < map->bloc_w && map->bmap((x >> 1) + 1, y >> 1) < map->nbbloc)
				{
					map->map_data(x, y).setLava(map->bloc[ map->bmap((x >> 1) + 1, y >> 1) ].lava);
					if (!map->map_data(x, y).isLava() && (y>>1) + 1 < map->bloc_h && map->bmap( (x >> 1) + 1, (y >> 1) + 1) < map->nbbloc)
						map->map_data(x, y).setLava( map->bloc[ map->bmap( (x >> 1) + 1, (y >> 1) + 1) ].lava );
					if (!map->map_data(x, y).isLava() && (y>>1) - 1 >= 0 && map->bmap((x >> 1) + 1, (y >> 1) - 1) < map->nbbloc)
						map->map_data(x, y).setLava( map->bloc[ map->bmap((x >> 1) + 1, (y >> 1) - 1) ].lava );
				}
				if (!map->map_data(x, y).isLava() && (x>>1) - 1 >= 0 && map->bmap((x >> 1) - 1, y >> 1) < map->nbbloc)
				{
					map->map_data(x, y).setLava( map->bloc[ map->bmap((x >> 1) - 1, y >> 1) ].lava );
					if (!map->map_data(x, y).isLava() && (y>>1) + 1 < map->bloc_h && map->bmap( (x >> 1) - 1, (y >> 1) + 1) < map->nbbloc)
						map->map_data(x, y).setLava( map->bloc[ map->bmap( (x >> 1) - 1, (y >> 1) + 1) ].lava );
					if (!map->map_data(x, y).isLava() && (y>>1) - 1 >= 0 && map->bmap((x >> 1) - 1, (y >> 1) - 1) < map->nbbloc)
						map->map_data(x, y).setLava( map->bloc[ map->bmap((x >> 1) - 1, (y >> 1) - 1) ].lava );
				}
				if (!map->map_data(x, y).isLava() && (y>>1) + 1 < map->bloc_h && map->bmap(x >> 1, (y >> 1) + 1) < map->nbbloc)
					map->map_data(x, y).setLava( map->bloc[ map->bmap(x >> 1, (y >> 1) + 1) ].lava );
				if (!map->map_data(x, y).isLava() && (y>>1) - 1 >= 0 && map->bmap(x >> 1, (y >> 1) - 1) < map->nbbloc)
					map->map_data(x, y).setLava( map->bloc[ map->bmap(x >> 1, (y >> 1) - 1) ].lava );
			}
        });

		LOG_INFO("Env created in " << float(msec_timer - event_timer) * 0.001f << "s.");
		event_timer = msec_timer;

		LOG_DEBUG("MAP: computing height data (step 3)");
		for (y = 0; y < (map->bloc_h << 1); ++y)
		{
			for (x = 0; x < (map->bloc_w << 1); ++x)	// Projete la carte du relief
			{
				float h = map->ph_map(x, y);
				int rec_y = y + int(0.5f - tnt_transform * h / 16.f);
				for (int cur_y=rec_y+1;cur_y<=y;cur_y++)
				{
					if (cur_y >= 0)
						map->ph_map(x, cur_y) = -1.f; // Valeur non spécifiée (un trou que l'on comblera plus tard)
				}
				if (rec_y >= 0)
					map->ph_map(x, rec_y) = h;
			}
		}

		LOG_DEBUG("MAP: computing height data (step 4)");
		map->sea_dec = map->sealvl * tnt_transform * H_DIV; // Calcule le décalage nécessaire pour restituer les océans
		for (y = 0; y < (map->bloc_h << 1); ++y)
		{
			for (x = 0; x < (map->bloc_w << 1); ++x) // Lisse la carte du relief projeté
			{
                if (!y && Math::Equals(map->ph_map(x, y), -1.f))
				{
					int cy = 0;
                    while (Math::Equals(map->ph_map(x, cy), -1.f))
						++cy;
					const float h = map->ph_map(x, cy);
					cy = 0;
                    while (Math::Equals(map->ph_map(x, cy), -1.f))
					{
						map->ph_map(x, cy) = h;
						++cy;
					}
				}
				else
				{
                    if (Math::Equals(map->ph_map(x, y), -1.f))
					{
						float h1 = map->ph_map(x, y - 1);
						int cy = y;
                        while (cy < (map->bloc_h << 1) && Math::Equals(map->ph_map(x, cy), -1.f))
							++cy;

						if (cy >= (map->bloc_h << 1))
							cy = (map->bloc_h << 1) - 1;

						float h2 = map->ph_map(x, cy);
                        if (Math::Equals(h2, -1.f))
							h2 = h1;
						for (int ry = y; ry < cy; ++ry)
							map->ph_map(x, ry) = h1 + (h2 - h1) * float(ry - y + 1) / float(cy - y + 1);
					}
				}
			}
		}

		LOG_DEBUG("MAP: computing height data (step 5)");
        parallel_for<int>(0, map->bloc_h << 1, [&](const int y)	 // Compute slopes on the map using height map and projected data
		{
			for (int x = 0; x < (map->bloc_w << 1); ++x)
			{
				float dh = 0.0f;
				if (y > 0)
				{
					float dz = fabsf( map->get_zdec( x, y) - map->get_zdec( x, y - 1) + 8.0f);
                    dz = (Math::Zero(dz)) ? 100000000.f : (8.0f / dz);
                    dh = Math::Max(dh, std::fabs(float(map->h_map(x, y) - map->h_map(x, y - 1))) * dz);
				}

				if (y + 1 < (map->bloc_h << 1))
				{
					float dz = fabsf( map->get_zdec( x, y + 1) - map->get_zdec( x, y) + 8.0f);
                    dz = (Math::Zero(dz)) ? 100000000.f : (8.0f / dz);
                    dh = Math::Max(dh, std::fabs(float(map->h_map(x, y)-map->h_map(x, y + 1))) * dz);
				}

				if (x > 0)
                    dh = Math::Max(dh, std::fabs(float(map->h_map(x, y)-map->h_map(x - 1, y))));
				if (x + 1 < (map->bloc_w << 1))
                    dh = Math::Max(dh, std::fabs(float(map->h_map(x, y)-map->h_map(x + 1, y))));

				map->slope(x,y) = dh;
				map->energy(x,y) = dh;
			}
        });
		LOG_DEBUG("MAP: computing height data (step 6)");
		gaussianFilter(map->energy, 3.0f);

		LOG_INFO("relief : " << float(msec_timer - event_timer) * 0.001f << "s.");
		event_timer = msec_timer;

		LOG_DEBUG("MAP: reading map features data");
		// Ajoute divers éléments(végétation,...)
		file->seek(header.PTRmapattr + 1);
        for (y = 0; y < (map->bloc_h << 1); ++y)
            for (x = 0; x < (map->bloc_w << 1); ++x)
				map->map_data(x, y).stuff = -1;
		features.destroy();
        for (y = 0; y < (map->bloc_h << 1); ++y)
		{
			for (x = 0; x < (map->bloc_w << 1); ++x)
			{
				unsigned short type;
                READ(type);
				if (type <= header.tileanims)
				{
					Vector3D Pos;
					Pos.x = float((x << 3) - map->map_w_d) + 8.0f;
					Pos.z = float((y << 3) - map->map_h_d) + 8.0f;
					const Feature* const feature = feature_manager.getFeaturePointer(TDF_index[type]);
					if (feature && !feature->m3d)
						Pos.y = map->get_max_rect_h( x, y, feature->footprintx, feature->footprintz);
					else
						Pos.y = map->get_unit_h( Pos.x, Pos.z);
                    if (x + 1 < map->bloc_w_db && y + 1 < map->bloc_h_db)
                    {
						map->map_data(x + 1, y + 1).stuff = features.add_feature(Pos,TDF_index[type]);
						features.drawFeatureOnMap( map->map_data(x + 1, y + 1).stuff);                  // Feature index is checked by drawFeatureOnMap so this is secure
						if (map->map_data(x + 1, y + 1).stuff >= 0 && feature->model)		// Rotate only 3D models
                        {
							if (feature_manager.getFeaturePointer(TDF_index[type])->category == "trees"			// Randomize trees angle for more realism
								|| feature_manager.getFeaturePointer(TDF_index[type])->category == "rocks")		// Randomize rocks angle for more realism
								features.feature[ map->map_data(x + 1, y + 1).stuff ].angle = float(TA3D_RAND() % 360);
                        }
                    }
				}
				// Read 2 more bytes
                READ(type);
			}
		}
		LOG_INFO("Decors : " << float(msec_timer - event_timer) * 0.001f << "s.");

		/*--------------- code for low definition map (mega zoom) -------------------*/

		LOG_DEBUG("MAP: creating low definition geometry (step 1)");
		map->low_w = (map->map_w + 32) >> 6;
		map->low_h = (map->map_h + 32) >> 6;
		map->low_nb_idx = (2 + map->low_w * 2) * map->low_h;			// Draw this as GL_TRIANGLE_STRIP
		int low_nb_vtx = (map->low_w + 1) * (map->low_h + 1);
		map->low_vtx = new Vector3D[low_nb_vtx];
		Vector3D *tmp_vtx = new Vector3D[low_nb_vtx];
		map->low_vtx_flat = new Vector3D[low_nb_vtx];
		map->low_col = new uint8[low_nb_vtx*4];
		map->low_tcoord = new float[low_nb_vtx*2];
		map->low_index = new GLuint[map->low_nb_idx];
		i = 0;
        parallel_for<int>(0, map->low_h + 1, [&](const int y) // Build the mesh
		{
			int i = y * (map->low_w + 1);
			for (int x = 0 ; x <= map->low_w ; ++x)
			{
				map->low_vtx[i].x = (float(x) - 0.5f * float(map->low_w)) / float(map->low_w) * float(map->map_w);
				map->low_vtx[i].z = (float(y) - 0.5f * float(map->low_h)) / float(map->low_h) * float(map->map_h);
				int X = x * ((map->bloc_w << 1) - 1) / map->low_w;
				int Y = y * ((map->bloc_h << 1) - 1) / map->low_h;
				map->low_vtx[i].y =  map->get_nh(X,Y);
				map->low_vtx[i].z += map->get_zdec(X,Y);
				tmp_vtx[i] = map->low_vtx_flat[i] = map->low_vtx[i];
				map->low_vtx_flat[i].y = 0.0f;
				map->low_tcoord[i<<1] = float(x) / float(map->low_w);
				map->low_tcoord[(i<<1)+1] = float(y) / float(map->low_h);
				map->low_col[(i<<2)+3] = 255;
				++i;
			}
        });
		if (map->water)
		{
            parallel_for<int>(1, map->low_h, [&](const int y)	// Make sure we'll see what is above water
			{
				for (int x = 1 ; x < map->low_w ; ++x)
				{
					int i = x + y * (map->low_w + 1);
					if (tmp_vtx[ i ].y < map->sealvl
						&& ( tmp_vtx[i - 1].y > map->sealvl ||
							 tmp_vtx[i + 1].y > map->sealvl ||
							 tmp_vtx[i - map->low_w - 1].y > map->sealvl ||
							 tmp_vtx[i + map->low_w + 1].y > map->sealvl))
						map->low_vtx[i].y = map->sealvl;
				}
            });
		}
		DELETE_ARRAY(tmp_vtx);
		LOG_DEBUG("MAP: creating low definition geometry (step 2)");
		i=0;
		for (y = 0; y < map->low_h; ++y)	// Build the mesh
		{
			for(x = 0; x < map->low_w; ++x)
			{
				if ((y&1) == 0)
				{
					if (x == 0)
					{
						map->low_index[i++]=y*(map->low_w+1)+x;
						map->low_index[i++]=(y+1)*(map->low_w+1)+x;
					}
					map->low_index[i++]=y*(map->low_w+1)+x+1;
					map->low_index[i++]=(y+1)*(map->low_w+1)+x+1;
				}
				else
				{
					if (x == 0)
					{
						map->low_index[i++]=y*(map->low_w+1)+map->low_w-x;
						map->low_index[i++]=(y+1)*(map->low_w+1)+map->low_w-x;
					}
					map->low_index[i++]=y*(map->low_w+1)+map->low_w-x-1;
					map->low_index[i++]=(y+1)*(map->low_w+1)+map->low_w-x-1;
				}
			}
		}

		/*---------------------------------------------------------------------------*/

		LOG_DEBUG("MAP: freeing temporary allocated memory");

		DELETE_ARRAY(TDF_index);
		DELETE_ARRAY(bmp_tex);

		return map;
	}



    GLuint load_tnt_minimap(QIODevice *file, int& sw, int& sh)		// Charge une minimap d'une carte, extraite d'une archive HPI/UFO
	{
		TNTHEADER header;
        READ(header);
		file->seek(header.PTRminimap);
		TNTMINIMAP minimap;
        READ(minimap);
        QImage bitmap = load_tnt_minimap_bmp(&minimap, &sw, &sh);

		if (g_useTextureCompression && lp_CONFIG->use_texture_compression)
			gfx->set_texture_format(GL_COMPRESSED_RGB_ARB);
		else
			gfx->set_texture_format(gfx->defaultTextureFormat_RGB());
        return gfx->make_texture(bitmap, FILTER_LINEAR, true);
	}

    GLuint load_tnt_minimap_fast(const QString& filename, int& sw,int& sh)		// Charge une minimap d'une carte contenue dans une archive HPI/UFO
	{
        QImage bitmap = load_tnt_minimap_fast_raw_bmp(filename, sw, sh);

        if (bitmap.isNull())    return 0;

		// Convert to a GL texture
		if (g_useTextureCompression && lp_CONFIG->use_texture_compression)
			gfx->set_texture_format(GL_COMPRESSED_RGB_ARB);
		else
			gfx->set_texture_format(gfx->defaultTextureFormat_RGB());
        return gfx->make_texture(bitmap, FILTER_LINEAR, true);
	}

    QImage load_tnt_minimap_fast_bmp(const QString& filename)		// Load a minimap into a QImage structure from a HPI/UFO archive
	{
		int sw, sh;
        QImage fullsize = load_tnt_minimap_fast_raw_bmp(filename, sw, sh);

        if (fullsize.isNull())    return QImage();

		// Copy the full-sized bitmap down to an exact-sized version
        QImage trimmed = gfx->create_surface(sw, sh);
		blit(fullsize,trimmed,0,0,0,0,sw,sh);

		return trimmed;
	}


} // namespace TA3D
