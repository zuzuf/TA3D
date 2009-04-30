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
#include "3do.h"
#include "EngineClass.h"
#include "tdf.h"
#include "tnt.h"
#include "misc/math.h"
#include "logs/logs.h"




namespace TA3D
{

	namespace
	{

		SDL_Surface *load_tnt_minimap_bmp(TNTMINIMAP *minimap,int *sw,int *sh)
		{
			// Copy the mini-map into an 8-bit SDL_Surface
			SDL_Surface *mini = gfx->create_surface_ex(8,TNTMINIMAP_WIDTH,TNTMINIMAP_HEIGHT);
			for(int y = 0; y < TNTMINIMAP_HEIGHT; ++y)
				memcpy((char*)mini->pixels + y * mini->pitch, minimap->map[y],TNTMINIMAP_WIDTH);

			// Apply the palette -- increase the color depth
			mini = convert_format(mini);

			// Examine the image for a blank-looking bottom or right edge
			int mini_w=TNTMINIMAP_WIDTH;
			int mini_h=TNTMINIMAP_HEIGHT;
			int blank_color = makecol(120,148,252); // approximately
			int mask = makecol(0xFC, 0xFC, 0xFC); // XXX this assumes 24- or 32-bit pixels
			do
			{
				--mini_w;
			} while ( mini_w > 0 &&
					( ( SurfaceInt(mini,mini_w,0) & mask) == blank_color ||
						SurfaceInt(mini,mini_w,0)         == 0));
			do
			{
				--mini_h;
			} while( mini_h > 0 &&
				( ( SurfaceInt(mini,0,mini_h) & mask) == blank_color ||
				SurfaceInt(mini,0,mini_h)         == 0));
			mini_w++;
			mini_h++;

			if (sw) *sw=mini_w;
			if (sh) *sh=mini_h;

			return mini;
		}

		SDL_Surface *load_tnt_minimap_fast_raw_bmp(const String& filename, int& sw, int& sh)
		{
			byte *headerBytes = HPIManager->PullFromHPI_zone(filename.c_str(),0,sizeof(TNTHEADER),NULL);
			if (headerBytes==NULL)
			{
				return 0;
			}
			TNTHEADER *header = &((TNTHEADER_U*)headerBytes)->header;

			byte *minimapdata = HPIManager->PullFromHPI_zone(filename.c_str(),header->PTRminimap,sizeof(TNTMINIMAP),NULL);
			if (minimapdata==NULL)
			{
				delete[] headerBytes;
				return 0;
			}

			TNTMINIMAP *minimap = &((TNTMINIMAP_U*)(&minimapdata[header->PTRminimap]))->map;
			SDL_Surface		*bitmap = load_tnt_minimap_bmp(minimap, &sw, &sh);

			delete[] headerBytes;
			delete[] minimapdata;

			return bitmap;
		}


	} // unnamed namespace






	MAP	*load_tnt_map(byte *data)		// Charge une map au format TA, extraite d'une archive HPI/UFO
	{
		LOG_DEBUG("MAP: creating MAP object ...");
		MAP	*map = new MAP;		// Crée une nouvelle carte

		the_map = map;

		LOG_DEBUG("MAP: initializing map data ...");

		map->init();
		map->tnt = true;
		TNTHEADER	header;		// Structure pour l'en-tête du fichier

		LOG_DEBUG("MAP: reading header");

		header.IDversion   = ((int*)data)[0];
		header.Width       = ((int*)data)[1];
		header.Height      = ((int*)data)[2];
		header.PTRmapdata  = ((int*)data)[3];
		header.PTRmapattr  = ((int*)data)[4];
		header.PTRtilegfx  = ((int*)data)[5];
		header.tiles       = ((int*)data)[6];
		header.tileanims   = ((int*)data)[7];
		header.PTRtileanim = ((int*)data)[8];
		header.sealevel    = ((int*)data)[9];
		header.PTRminimap  = ((int*)data)[10];
		header.unknown1    = ((int*)data)[11];
		header.pad1        = ((int*)data)[12];
		header.pad2        = ((int*)data)[13];
		header.pad3        = ((int*)data)[14];
		header.pad4        = ((int*)data)[15];

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
			TDF_index[i]=feature_manager.get_feature_index((char*)(data+header.PTRtileanim+4+(i*132)));
			if (TDF_index[i] == -1)
				LOG_ERROR("tdf not found: " << (char*)(data + header.PTRtileanim + 4 + (i * 132)));
		}

		map->sealvl = header.sealevel * H_DIV;
		int f_pos;
		// Lit la minimap
		LOG_DEBUG("MAP: reading mini map");
		int event_timer = msec_timer;
		int w,h;
		f_pos=header.PTRminimap;
		w = *((int*)(data + f_pos)); f_pos += 4;
		h = *((int*)(data + f_pos)); f_pos += 4;
		map->mini_w = w;
		map->mini_h = h;
		map->mini = gfx->create_surface_ex(8, 252, 252);
		for(y = 0; y < 252; ++y)
		{
			memcpy((char*)map->mini->pixels + y * map->mini->pitch, data + f_pos, 252);
			f_pos += 252;
		}
		map->mini = convert_format(map->mini);
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
			gfx->set_texture_format(GL_RGB8);
		map->glmini = gfx->make_texture(map->mini,FILTER_LINEAR,true);

		LOG_INFO("minimap read in " << (msec_timer - event_timer) * 0.001f << "s.");

		// Lit les différents morceaux
		LOG_DEBUG("MAP: reading blocs data");
		event_timer = msec_timer;
		int n_bmp = (header.tiles+0x3F) >> 5; // Nombre de textures 1024x32 nécessaires pour mémoriser tout les morceaux
		SDL_Surface **bmp_tex = new SDL_Surface*[n_bmp];
		for (i = 0; i < n_bmp; ++i)
			bmp_tex[i] = gfx->create_surface_ex(8, 1024, 32);

		f_pos=header.PTRtilegfx;
		for(i = 0; i < header.tiles; ++i) // Lit tout les morceaux
		{
			int tex_num = i>>5;	// Numéro de la texture associée
			int tx = (i&0x1F)<<5;			// Coordonnées sur la texture
			for(y = 0; y < 32; ++y)	// Lit le morceau
			{
				memcpy((char*)bmp_tex[tex_num]->pixels + y * bmp_tex[tex_num]->pitch + tx,data+f_pos,32);
				f_pos += 32;
			}
		}

		LOG_DEBUG("MAP: allocating map memory");
		map->bloc_w = header.Width>>1;
		map->bloc_h = header.Height>>1;
		map->bloc_w_db = map->bloc_w<<1;
		map->bloc_h_db = map->bloc_h<<1;
		map->map_h = map->bloc_h<<4;
		map->map_w = map->bloc_w<<4;
		map->map_h_d = map->bloc_h<<3;
		map->map_w_d = map->bloc_w<<3;
		map->map2blocdb_w = ((float)map->bloc_w_db)/map->map_w;
		map->map2blocdb_h = ((float)map->bloc_h_db)/map->map_h;
		map->bmap = new unsigned short*[map->bloc_h];
		map->bmap[0] = new unsigned short[map->bloc_w*map->bloc_h];
		for(i = 1; i < map->bloc_h; ++i)
			map->bmap[i] = &(map->bmap[0][i*map->bloc_w]);
		map->view = new byte*[map->bloc_h];
		map->view[0] = new byte[map->bloc_w*map->bloc_h];
		map->path = new byte*[map->bloc_h<<2];
		map->path[0] = new byte[map->bloc_w*map->bloc_h<<4];
		map->map_data = new SECTOR*[map->bloc_h<<1];
		map->map_data[0] = new SECTOR[map->bloc_w*map->bloc_h<<2];

		LOG_DEBUG("MAP: creating FOW maps");
		map->sight_map = gfx->create_surface_ex( 8, map->bloc_w, map->bloc_h);		// FOW maps
		map->view_map  = gfx->create_surface_ex( 8, map->bloc_w, map->bloc_h);
		map->radar_map = gfx->create_surface_ex( 8, map->bloc_w, map->bloc_h);
		map->sonar_map = gfx->create_surface_ex( 8, map->bloc_w, map->bloc_h);
		SDL_FillRect( map->view_map, NULL, 0);
		SDL_FillRect( map->sight_map, NULL, 0);
		SDL_FillRect( map->radar_map, NULL, 0);
		SDL_FillRect( map->sonar_map, NULL, 0);

		LOG_DEBUG("MAP: allocating height maps");
		map->h_map       = new float*[map->bloc_h<<1];
		map->h_map[0]    = new float[map->bloc_w*map->bloc_h<<2];
		map->ph_map      = new float*[map->bloc_h<<1];
		map->ph_map[0]   = new float[map->bloc_w*map->bloc_h<<2];
		map->ph_map_2    = new byte*[map->bloc_h<<1];
		map->ph_map_2[0] = new byte[map->bloc_w*map->bloc_h<<2];

		LOG_DEBUG("MAP: initialising map data");
		for (i = 1; i < (map->bloc_h << 2); ++i)
			map->path[i] = &(map->path[0][i*map->bloc_w<<2]);
		memset(map->path[0],0,map->bloc_w*map->bloc_h<<4);
		for (i = 1; i < (map->bloc_h << 1); ++i)
		{
			map->h_map[i] = &(map->h_map[0][i*map->bloc_w<<1]);
			map->ph_map[i] = &(map->ph_map[0][i*map->bloc_w<<1]);
			map->ph_map_2[i] = &(map->ph_map_2[0][i*map->bloc_w<<1]);
			map->map_data[i] = &(map->map_data[0][i*map->bloc_w<<1]);
			if (i<map->bloc_h)
				map->view[i] = &(map->view[0][i*map->bloc_w]);
		}

		memset(map->view[0],0,map->bloc_w*map->bloc_h);
		map->nbbloc = header.tiles;		// Nombre de blocs nécessaires
		map->bloc = new BLOC[map->nbbloc];	// Alloue la mémoire pour les blocs
		map->ntex = n_bmp;
		map->tex = new GLuint[n_bmp];	// Tableau d'indices de texture OpenGl

		for (i = 0 ; i < map->nbbloc ; i++) // Crée les blocs
		{
			map->bloc[i].init();
			int tex_num = i>>5;	// Numéro de la texture associée
			int tx = (i&0x1F)<<5;			// Coordonnées sur la texture
			int r=0,g=0,b=0;
			for (y = 0; y < 32; ++y)
			{
				for (x = tx; x < tx + 32; ++x)
				{
					int c = SurfaceByte(bmp_tex[tex_num],x,y);
					r += pal[c].r;
					g += pal[c].g;
					b += pal[c].b;
				}
			}
			r >>= 10;
			g >>= 10;
			b >>= 10;
			map->bloc[i].lava=(r>4 && g<(r>>2) && b<(r>>2));
			map->bloc[i].tex_x = tx>>5;
		}

		LOG_INFO("Blocs read in " << (msec_timer-event_timer) * 0.001f << "s.");
		event_timer = msec_timer;

		LOG_DEBUG("MAP: creating textures");

		if (g_useTextureCompression && lp_CONFIG->use_texture_compression)
			gfx->set_texture_format(GL_COMPRESSED_RGB_ARB);
		else
			gfx->set_texture_format(GL_RGB8);
		for (i = 0; i < n_bmp; ++i) // Finis de charger les textures et détruit les objets SDL_Surface
		{
			bmp_tex[i] = convert_format_24( bmp_tex[i] );
			map->tex[i] = gfx->make_texture(bmp_tex[i]);
		}
		LOG_INFO("Textures for blocks in " << (msec_timer - event_timer) * 0.001f << "s.");

		event_timer = msec_timer;

		map->lvl = new Vector3D*[map->bloc_w*map->bloc_h];
		for (i = 0; i < map->bloc_w * map->bloc_h; ++i)
			map->lvl[i] = NULL;

		LOG_DEBUG("MAP: creating blocs texture coordinates");
		for(i = 0; i < map->nbbloc; ++i) // Crée les blocs
		{
			int t_n=i>>5;				// Numéro de texture
			float t_x=((float)(i&0x1F))/32.0f;	// Position sur la texture

			map->bloc[i].tex = map->tex[t_n];
			map->bloc[i].nbpoint = 9;
			map->bloc[i].nbindex = 12;
			map->bloc[i].texcoord = new float[map->bloc[i].nbpoint<<1];

			float c = 1.0f/32.0f-1.0f/1024.0f;
			t_x+=1.0f/2048.0f;
			map->bloc[i].texcoord[0]=t_x;				map->bloc[i].texcoord[1]=1.0f/64.0f;
			map->bloc[i].texcoord[2]=t_x+c*0.5f;		map->bloc[i].texcoord[3]=1.0f/64.0f;
			map->bloc[i].texcoord[4]=t_x+c;				map->bloc[i].texcoord[5]=1.0f/64.0f;
			map->bloc[i].texcoord[6]=t_x;				map->bloc[i].texcoord[7]=0.5f;
			map->bloc[i].texcoord[8]=t_x+c*0.5f;		map->bloc[i].texcoord[9]=0.5f;
			map->bloc[i].texcoord[10]=t_x+c;			map->bloc[i].texcoord[11]=0.5f;
			map->bloc[i].texcoord[12]=t_x;				map->bloc[i].texcoord[13]=63.0f/64.0f;
			map->bloc[i].texcoord[14]=t_x+c*0.5f;		map->bloc[i].texcoord[15]=63.0f/64.0f;
			map->bloc[i].texcoord[16]=t_x+c;			map->bloc[i].texcoord[17]=63.0f/64.0f;
		}

		// Charge les données sur la position des blocs
		int max_tex_size;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE,&max_tex_size);

		/*---------- code to convert the map to new format -------------------*/
		/*	map->macro_w = map->bloc_w+15>>4;
			map->macro_h = map->bloc_h+15>>4;
			map->macro_bloc = new BLOC*[map->macro_h];
			map->macro_bloc[0] = new BLOC[map->macro_h*macro_w];

			for(i=1;i<map->macro_h;i++)
			map->macro_bloc[i] = &(map->macro_bloc[0][macro_w*i]);

			SDL_Surface *tmp = gfx->create_surface_ex(32,512,512);
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

		SDL_Surface *low_def = gfx->create_surface_ex(24, Math::Min(max_tex_size,map->map_w), Math::Min(max_tex_size,map->map_h));
		SDL_FillRect(low_def, NULL, 0x0);
		SDL_Surface *lava_map = gfx->create_surface_ex(24, Math::Min(map->bloc_w,1024), Math::Min(map->bloc_h,1024));
		SDL_FillRect(lava_map, NULL, 0x0);
		f_pos = header.PTRmapdata;
		for (y = 0; y < map->bloc_h; ++y)
		{
			for (x = 0; x < map->bloc_w; ++x)
			{
				map->bmap[y][x] = *((short*)(data+f_pos));

				if (map->bmap[y][x] >= map->nbbloc)		// To add some security
					map->bmap[y][x] = 0;

				/*---------- code to build the low def map (mega zoom) ---------------*/
				i=map->bmap[y][x];
				int tex_num=i>>5;	// Numéro de la texture associée
				int tx=(i&0x1F)<<5;			// Coordonnées sur la texture

				if (bmp_tex[tex_num]->format->BitsPerPixel != 24)
					bmp_tex[tex_num] = convert_format_24(bmp_tex[tex_num]);

				stretch_blit(bmp_tex[tex_num], low_def, tx, 0, 32, 32,
							 x * (low_def->w - 1) / map->bloc_w, y * (low_def->h - 1) / map->bloc_h,
							 (x + 1) * (low_def->w - 1) / map->bloc_w - x * (low_def->w - 1) / map->bloc_w,
							 (y + 1) * (low_def->h - 1) / map->bloc_h - y * (low_def->h - 1) / map->bloc_h);
				/*--------------------------------------------------------------------*/

				if (map->bloc[map->bmap[y][x]].lava)
					circlefill(lava_map,x*lava_map->w/map->bloc_w,y*lava_map->h/map->bloc_h,3,0xFFFFFFFF);
				f_pos+=2;
			}
		}
		for (i = 0; i < n_bmp; ++i)				// Delete SDL_Surface textures
			SDL_FreeSurface(bmp_tex[i]);
		map->low_tex = gfx->make_texture(low_def);		// Build the low details texture map
		SDL_FreeSurface(low_def);

		map->lava_map = gfx->make_texture(lava_map,FILTER_LINEAR,true);		// Build the lava texture map
		SDL_FreeSurface(lava_map);

		LOG_DEBUG("MAP: computing height data (step 1)");
		// Charge d'autres données sur les blocs
		map->water = false;
		f_pos = header.PTRmapattr;
		for (y = 0; y< (map->bloc_h << 1); ++y)
		{
			for (x = 0; x < (map->bloc_w << 1);  ++x)
			{
				int c = *((byte*)(data+f_pos));
				if (c<header.sealevel) map->water = true;
				map->h_map[y][x] = map->ph_map[y][x] = c*H_DIV;
				f_pos += 4;
			}
		}

		LOG_DEBUG("MAP: computing height data (step 2)");
		for(y=0;y<(map->bloc_h<<1);y++)				// Calcule les informations complémentaires sur la carte
		{
			for (x = 0; x < (map->bloc_w << 1); ++x)
			{
				map->map_data[y][x].init();
				map->map_data[y][x].underwater = (map->h_map[y][x] < map->sealvl);
				map->map_data[y][x].lava = map->bmap[ y >> 1 ][ x >> 1 ] < map->nbbloc ? map->bloc[ map->bmap[ y >> 1 ][ x >> 1 ] ].lava : false;
				if (!map->map_data[y][x].lava && (x>>1) + 1 < map->bloc_w && map->bmap[ y >> 1 ][ (x >> 1) + 1 ] < map->nbbloc)
				{
					map->map_data[y][x].lava = map->bloc[ map->bmap[ y >> 1 ][ (x >> 1) + 1 ] ].lava;
					if (!map->map_data[y][x].lava && (y>>1) + 1 < map->bloc_h && map->bmap[ (y >> 1) + 1 ][ (x >> 1) + 1 ] < map->nbbloc)
						map->map_data[y][x].lava = map->bloc[ map->bmap[ (y >> 1) + 1 ][ (x >> 1) + 1 ] ].lava;
					if (!map->map_data[y][x].lava && (y>>1) - 1 >= 0 && map->bmap[ (y >> 1) - 1 ][ (x >> 1) + 1 ] < map->nbbloc)
						map->map_data[y][x].lava = map->bloc[ map->bmap[ (y >> 1) - 1 ][ (x >> 1) + 1 ] ].lava;
				}
				if (!map->map_data[y][x].lava && (x>>1) - 1 >= 0 && map->bmap[ y >> 1 ][ (x >> 1) - 1 ] < map->nbbloc)
				{
					map->map_data[y][x].lava = map->bloc[ map->bmap[ y >> 1 ][ (x >> 1) - 1 ] ].lava;
					if (!map->map_data[y][x].lava && (y>>1) + 1 < map->bloc_h && map->bmap[ (y >> 1) + 1 ][ (x >> 1) - 1 ] < map->nbbloc)
						map->map_data[y][x].lava = map->bloc[ map->bmap[ (y >> 1) + 1 ][ (x >> 1) - 1 ] ].lava;
					if (!map->map_data[y][x].lava && (y>>1) - 1 >= 0 && map->bmap[ (y >> 1) - 1 ][ (x >> 1) - 1 ] < map->nbbloc)
						map->map_data[y][x].lava = map->bloc[ map->bmap[ (y >> 1) - 1 ][ (x >> 1) - 1 ] ].lava;
				}
				if (!map->map_data[y][x].lava && (y>>1) + 1 < map->bloc_h && map->bmap[ (y >> 1) + 1 ][ x >> 1 ] < map->nbbloc)
					map->map_data[y][x].lava = map->bloc[ map->bmap[ (y >> 1) + 1 ][ x >> 1 ] ].lava;
				if (!map->map_data[y][x].lava && (y>>1) - 1 >= 0 && map->bmap[ (y >> 1) - 1 ][ x >> 1 ] < map->nbbloc)
					map->map_data[y][x].lava = map->bloc[ map->bmap[ (y >> 1) - 1 ][ x >> 1 ] ].lava;
			}
		}

		LOG_INFO("Env created in " << (msec_timer-event_timer) * 0.001f << "s.");
		event_timer = msec_timer;

		LOG_DEBUG("MAP: computing height data (step 3)");
		for (y = 0; y < (map->bloc_h << 1); ++y)
		{
			for(x = 0; x < (map->bloc_w << 1); ++x)	// Projete la carte du relief
			{
				int rec_y;
				float h=map->ph_map[y][x];
				rec_y=(int)(0.5f+y-tnt_transform*h/16.0f);
				for(int cur_y=rec_y+1;cur_y<=y;cur_y++)
					if (cur_y>=0)
						map->ph_map[cur_y][x]=-1.0f;		// Valeur non spécifiée (un trou que l'on comblera plus tard)
				if (rec_y>=0)
					map->ph_map[rec_y][x]=h;
			}
		}

		LOG_DEBUG("MAP: computing height data (step 4)");
		map->sea_dec=map->sealvl*tnt_transform*H_DIV;			// Calcule le décalage nécessaire pour restituer les océans
		for (y = 0; y < (map->bloc_h << 1); ++y)
		{
			for(x = 0; x < (map->bloc_w << 1); ++x) // Lisse la carte du relief projeté
			{
				if (map->ph_map[y][x] == -1.0f && y == 0)
				{
					int cy = 0;
					while (map->ph_map[cy][x]==-1.0f)
						++cy;
					float h=map->ph_map[cy][x];
					cy=0;
					while (map->ph_map[cy][x] == -1.0f)
					{
						map->ph_map[cy][x]=h;
						cy++;
					}
				}
				else
				{
					if (map->ph_map[y][x]==-1.0f)
					{
						float h1=map->ph_map[y-1][x];
						int cy=y;
						while (cy < (map->bloc_h << 1) && map->ph_map[cy][x] == -1.0f)
							++cy;
						if (cy>=(map->bloc_h<<1)) cy=(map->bloc_h<<1)-1;
						float h2=map->ph_map[cy][x];
						if (h2 == -1.0f)
							h2=h1;
						for (int ry = y; ry < cy; ++ry)
							map->ph_map[ry][x] = h1+(h2-h1) * (ry-y+1) / (cy-y+1);
					}
				}
			}
		}
		LOG_DEBUG("MAP: computing height data (step 5)");
		for(y=0;y<(map->bloc_h<<1);y++)
			for(x=0;x<(map->bloc_w<<1);x++)				// Compute the second map
				map->ph_map_2[y][x]=(byte)(map->ph_map[y][x]*0.125f*tnt_transform_H_DIV+0.5f);

		LOG_DEBUG("MAP: computing height data (step 6)");
		for (y = 0;y < (map->bloc_h << 1); ++y)	 // Compute slopes on the map using height map and projected datas
		{
			for (x = 0; x < (map->bloc_w << 1); ++x)
			{
				float dh=0.0f;
				if (y > 0)
				{
					float dz = fabsf( map->get_zdec( x, y) - map->get_zdec( x, y - 1) + 8.0f);
					if (dz == 0.0f)	dz = 100000000.0f;
					else			dz = 8.0f / dz;
					dh = Math::Max(dh,(float)fabsf(map->h_map[y][x]-map->h_map[y-1][x]) * dz);
				}
				if (y + 1 < (map->bloc_h << 1))
				{
					float dz = fabsf( map->get_zdec( x, y + 1) - map->get_zdec( x, y) + 8.0f);
					if (dz == 0.0f)	dz = 100000000.0f;
					else				dz = 8.0f / dz;
					dh = Math::Max(dh,(float)fabsf(map->h_map[y][x]-map->h_map[y+1][x]) * dz);
				}
				if (x > 0)
					dh = Math::Max(dh,(float)fabsf(map->h_map[y][x]-map->h_map[y][x-1]));
				if (x + 1 < (map->bloc_w << 1))
					dh = Math::Max(dh,(float)fabsf(map->h_map[y][x]-map->h_map[y][x+1]));
				map->map_data[y][x].dh=dh;
			}
		}

		LOG_INFO("relief : " << (msec_timer - event_timer) * 0.001f << "s.");
		event_timer = msec_timer;

		LOG_DEBUG("MAP: reading map features data");
		// Ajoute divers éléments(végétation,...)
		f_pos=header.PTRmapattr+1;
		for (y = 0; y < (map->bloc_h << 1); ++y)
		{
			for (x = 0; x < (map->bloc_w << 1); ++x)
			{
				unsigned short type = *((unsigned short*)(data + f_pos));
				map->map_data[y][x].stuff = -1;
				if (type <= header.tileanims)
				{
					Vector3D Pos;
					Pos.x = (x<<3) - map->map_w_d + 8.0f;
					Pos.z = (y<<3) - map->map_h_d + 8.0f;
					Feature *feature = feature_manager.getFeaturePointer(TDF_index[type]);
					if (feature && !feature->m3d)
						Pos.y = map->get_max_rect_h( x, y, feature->footprintx, feature->footprintz);
					else
						Pos.y = map->get_unit_h( Pos.x, Pos.z);
					map->map_data[y][x].stuff = features.add_feature(Pos,TDF_index[type]);
					features.drawFeatureOnMap( map->map_data[y][x].stuff);                  // Feature index is checked by drawFeatureOnMap so this is secure
				}
				f_pos+=4;
			}
		}
		LOG_INFO("Decors : " << (msec_timer - event_timer) * 0.001f << "s.");

		/*--------------- code for low definition map (mega zoom) -------------------*/

		LOG_DEBUG("MAP: creating low definition geometry (step 1)");
		map->low_w=(map->map_w+32)>>6;
		map->low_h=(map->map_h+32)>>6;
		map->low_nb_idx = (2+map->low_w*2)*map->low_h;			// Draw this as GL_TRIANGLE_STRIP
		int low_nb_vtx = (map->low_w+1)*(map->low_h+1);
		map->low_vtx = new Vector3D[low_nb_vtx];
		Vector3D *tmp_vtx = new Vector3D[low_nb_vtx];
		map->low_vtx_flat = new Vector3D[low_nb_vtx];
		map->low_col = new uint8[low_nb_vtx*4];
		map->low_tcoord = new float[low_nb_vtx*2];
		map->low_index = new GLuint[map->low_nb_idx];
		i = 0;
		for (y = 0; y <= map->low_h; ++y) // Build the mesh
		{
			for (x = 0; x <= map->low_w; ++x)
			{
				map->low_vtx[i].x = (x - 0.5f * map->low_w) / map->low_w * map->map_w;
				map->low_vtx[i].z = (y - 0.5f * map->low_h) / map->low_h * map->map_h;
				int X = x * ((map->bloc_w << 1) - 1) / map->low_w;
				int Y = y * ((map->bloc_h << 1) - 1) / map->low_h;
				map->low_vtx[i].y =  map->get_nh(X,Y);
				map->low_vtx[i].z += map->get_zdec(X,Y);
				tmp_vtx[i] = map->low_vtx_flat[i] = map->low_vtx[i];
				map->low_vtx_flat[i].y = 0.0f;
				map->low_tcoord[i<<1] = ((float)x) / map->low_w;
				map->low_tcoord[(i<<1)+1] = ((float)y) / map->low_h;
				map->low_col[(i<<2)+3] = 255;
				++i;
			}
		}
		if (map->water)
		{
			for (y = 1 ; y < map->low_h ; ++y)	// Make sure we'll see what is above water
			{
				for( x = 1 ; x < map->low_w ; ++x)
				{
					i = x + y * (map->low_w + 1);
					if (tmp_vtx[ i ].y < map->sealvl
						&& ( tmp_vtx[i - 1].y > map->sealvl ||
							 tmp_vtx[i + 1].y > map->sealvl ||
							 tmp_vtx[i - map->low_w - 1].y > map->sealvl ||
							 tmp_vtx[i + map->low_w + 1].y > map->sealvl))
						map->low_vtx[i].y = map->sealvl;
				}
			}
		}
		delete[] tmp_vtx;
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

		delete[] TDF_index;
		delete[] bmp_tex;

		return map;
	}



	GLuint load_tnt_minimap(byte *data,int& sw,int& sh)		// Charge une minimap d'une carte, extraite d'une archive HPI/UFO
	{
		TNTHEADER	*header = (TNTHEADER*)data;
		TNTMINIMAP *minimap = (TNTMINIMAP*) &data[header->PTRminimap];
		SDL_Surface		*bitmap = load_tnt_minimap_bmp(minimap, &sw, &sh);

		if (g_useTextureCompression && lp_CONFIG->use_texture_compression)
			gfx->set_texture_format(GL_COMPRESSED_RGB_ARB);
		else
			gfx->set_texture_format(GL_RGB8);
		GLuint texture = gfx->make_texture(bitmap, FILTER_LINEAR, true);

		SDL_FreeSurface(bitmap);
		return texture;
	}

	GLuint load_tnt_minimap_fast(const String& filename, int& sw,int& sh)		// Charge une minimap d'une carte contenue dans une archive HPI/UFO
	{
		SDL_Surface *bitmap = load_tnt_minimap_fast_raw_bmp(filename, sw, sh);

		if (bitmap == NULL)    return 0;

		// Convert to a GL texture
		if (g_useTextureCompression && lp_CONFIG->use_texture_compression)
			gfx->set_texture_format(GL_COMPRESSED_RGB_ARB);
		else
			gfx->set_texture_format(GL_RGB8);
		GLuint texture = gfx->make_texture(bitmap, FILTER_LINEAR, true);

		SDL_FreeSurface(bitmap);
		return texture;
	}

	SDL_Surface *load_tnt_minimap_fast_bmp(const String& filename)		// Load a minimap into a SDL_Surface* structure from a HPI/UFO archive
	{
		int sw, sh;
		SDL_Surface *fullsize = load_tnt_minimap_fast_raw_bmp(filename, sw, sh);

		if (fullsize == NULL)    return 0;

		// Copy the full-sized bitmap down to an exact-sized version
		SDL_Surface *trimmed = gfx->create_surface(sw, sh);
		blit(fullsize,trimmed,0,0,0,0,sw,sh);
		SDL_FreeSurface(fullsize);

		return trimmed;
	}


} // namespace TA3D
