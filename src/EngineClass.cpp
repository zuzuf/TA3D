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
#include "3do.h"					// Pour la lecture des fichiers 3D
#include "scripts/cob.h"					// Pour la lecture et l'éxecution des scripts
#include "tdf.h"					// Pour la gestion des éléments du jeu
#include "EngineClass.h"
#include "UnitEngine.h"
#include "gfx/fx.h"
#include <list>
#include "misc/math.h"
#include "logs/logs.h"
#include "misc/tdf.h"



byte player_color_map[TA3D_PLAYERS_HARD_LIMIT] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };


float	player_color[TA3D_PLAYERS_HARD_LIMIT * 3]=
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





namespace TA3D
{

    MAP* the_map = NULL;




    void IDX_LIST::destroy()
    {
        while (head)
        {
            IDX_LIST_NODE* next = head->next;
            delete head;
            head = next;
        }
    }

    void IDX_LIST::push(const sint16 idx)
    {
        if (head)
        {
            IDX_LIST_NODE *cur = head;
            while (cur->next)
            {
                if (cur->idx == idx) // Don't add it twice
                    return;
                cur = cur->next;
            }
            cur->next = new IDX_LIST_NODE(idx); // Add idx at the end
        }
        else
            head = new IDX_LIST_NODE(idx);
    }


    void IDX_LIST::remove(const sint16 idx) // Assume there is only one occurence of idx in the list
    {
        IDX_LIST_NODE *cur = head;
        IDX_LIST_NODE *prec = NULL;
        while (cur)
        {
            if (cur->idx == idx)
            {
                if (prec == NULL)
                {
                    prec = head;
                    head = head->next;
                    delete prec;
                    return;
                }
                else
                {
                    prec->next = cur->next;
                    delete cur;
                    return;
                }
            }
            prec = cur;
            cur = cur->next;
        }
    }

    bool IDX_LIST::isIn(const sint16 idx)
    {
        IDX_LIST_NODE *cur = head;
        while (cur)
        {
            if (cur->idx == idx)
                return true;
            cur = cur->next;
        }
        return false;
    }


    void SECTOR::init()
    {
        dh = 0.0f;
        underwater = false;
        stuff = -1;
        unit_idx = -1;
        lava = false;
        air_idx.init();
        flat = false;
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
        for (short int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
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
        view_map = NULL;
        sight_map = NULL;
        radar_map = NULL;
        sonar_map = NULL;

        shadow2_shader.load("shaders/map_shadow.frag", "shaders/map_shadow.vert");
        detail_shader.load( "shaders/details.frag", "shaders/details.vert" );
        details_tex = 0;
        color_factor = 1.0f;

        low_nb_idx=0;
        low_vtx=NULL;
        low_vtx_flat=NULL;
        low_tcoord=NULL;
        low_col=NULL;
        low_index=NULL;
        low_tex=0;

        wind=0.0f;
        wind_dir=0.0f;
        wind_vec.x=wind_vec.y=wind_vec.z=0.0f;
        ota_data.init();
        lava_map=0;
        path=NULL;
        mini_w=mini_h=252;
        ntex=0;
        tex=NULL;
        nbbloc=0;
        bloc=NULL;
        mini=NULL;
        bmap=NULL;
        h_map=NULL;
        ph_map=NULL;
        ph_map_2=NULL;
        map_data=NULL;
        sealvl=0.0f;
        glmini=0;
        lvl=NULL;
        water=true;
        tnt=false;			// Laisse la possibilité de créer un autre format de cartes
        sea_dec=0.0f;
        view=NULL;
        ox1=ox2=oy1=oy2=0;
        short buf_size=0;
        for (short int i = 0; i < 6500; ++i)
        {
            buf_i[i++] = 0 + buf_size;
            buf_i[i++] = 1 + buf_size;
            buf_i[i++] = 3 + buf_size;
            buf_i[i++] = 4 + buf_size;
            buf_i[i++] = 6 + buf_size;
            buf_i[i++] = 7 + buf_size;
            buf_i[i++] = 7 + buf_size;
            buf_i[i++] = 8 + buf_size;
            buf_i[i++] = 4 + buf_size;
            buf_i[i++] = 5 + buf_size;
            buf_i[i++] = 1 + buf_size;
            buf_i[i++] = 2 + buf_size;
            buf_i[i]   = 2 + buf_size;
            buf_size+=9;
        }
    }



    float MAP::get_unit_h(float x,float y)
    {
        x=(x+map_w_d)*0.125f;		// Convertit les coordonnées
        y=(y+map_h_d)*0.125f;
        int lx = bloc_w_db-1;
        int ly = bloc_h_db-1;
        if(x<0.0f) x=0.0f;
        else if(x>=lx) x=bloc_w_db-2;
        if(y<0.0f) y=0.0f;
        else if(y>=ly) y=bloc_h_db-2;
        float h[4];
        int X=(int)x,Y=(int)y;
        float dx=x-X;
        float dy=y-Y;
        h[0]=h_map[Y][X];
        if(X+1<lx)
            h[1]=h_map[Y][X+1]-h[0];
        else
            h[1]=0.0f;

        if (Y+1<ly)
        {
            h[2]=h_map[Y+1][X];
            if(X+1<lx)
                h[3]=h_map[Y+1][X+1]-h[2];
            else
                h[3]=0.0f;
        }
        else
        {
            h[2]=h[0];
            h[3]=h[1];
        }
        h[0]=h[0]+h[1]*dx;
        return h[0]+(h[2]+h[3]*dx-h[0])*dy;
    }


    float MAP::get_h(int x,int y)
    {
        if(x<0) x=0;
        if(y<0) y=0;
        if(x>=bloc_w_db-1) x=bloc_w_db-2;
        if(y>=bloc_h_db-1) y=bloc_h_db-2;
        return h_map[y][x];
    }

    float MAP::get_max_h(int x,int y)
    {
        if(x<0) x=0;
        if(y<0) y=0;
        if(x>=bloc_w_db-1) x=bloc_w_db-2;
        if(y>=bloc_h_db-1) y=bloc_h_db-2;
        float h = h_map[y][x];
        if(x<bloc_w_db-2)	h = Math::Max(h, h_map[y][x+1]);
        if(y<bloc_h_db-2)
        {
            h = Math::Max(h, h_map[y+1][x]);
            if(x<bloc_w_db-2)
                h = Math::Max(h, h_map[y+1][x+1]);
        }
        return h;
    }


    float MAP::get_max_rect_h(int x,int y, int w, int h)
    {
        int x1 = x - (w>>1);
        int x2 = x1 + w;
        int y1 = y - (h>>1);
        int y2 = y1 + h;
        if(x1<0) x1=0;
        if(y1<0) y1=0;
        if(x1>=bloc_w_db-1) x1=bloc_w_db-2;
        if(y1>=bloc_h_db-1) y1=bloc_h_db-2;
        if(x2<0) x2=0;
        if(y2<0) y2=0;
        if(x2>=bloc_w_db-1) x2=bloc_w_db-2;
        if(y2>=bloc_h_db-1) y2=bloc_h_db-2;
        float max_h = h_map[y1][x1];
        for (int Y = y1; Y <= y2 ; ++Y)
            for (int X = x1; X <= x2 ; ++X)
            {
                float h = h_map[Y][X];
                if( h > max_h )	max_h = h;
            }
        return max_h;
    }

    float MAP::get_zdec(int x,int y)
    {
        if(x<0) x=0;
        if(y<0) y=0;
        if(x>=bloc_w_db-1) x=bloc_w_db-2;
        if(y>=bloc_h_db-1) y=bloc_h_db-2;
        return ph_map[y][x]*tnt_transform_H_DIV;
    }


    float MAP::get_nh(int x,int y)
    {
        if(x<0) x=0;
        if(y<0) y=0;
        if(x>=bloc_w_db-1) x=bloc_w_db-2;
        if(y>=bloc_h_db-1) y=bloc_h_db-2;
        return ph_map[y][x];
    }


    void MAP::rect(int x1,int y1,int w,int h,int c,const String &yardmap,bool open)
    {
        if (yardmap.empty())
        {
            int y2=y1+h;
            int x2=x1+w;
            if(y1<0)	y1=0;
            if(y2>bloc_h_db-1)	y2=bloc_h_db-1;
            if(x1<0)	x1=0;
            if(x2>bloc_w_db-1)	x2=bloc_w_db-1;
            if(y2<=y1 || x2<=x1)	return;
            pMutex.lock();
            for(int y=y1;y<y2;y++)
                for(int x=x1;x<x2;x++)
                    map_data[y][x].unit_idx=c;
            pMutex.unlock();
        }
        else
        {
            int i=0;
            int y2=y1+h;
            int x2=x1+w;
            if(y1<0)
            {
                i -= y1 * w;
                y1 = 0;
            }
            if(y2>bloc_h_db-1)	y2=bloc_h_db-1;
            if(x1<0)	x1=0;
            if(x2>bloc_w_db-1)	x2=bloc_w_db-1;
            int dw=w-(x2-x1);
            if(y2<=y1 || x2<=x1)	return;
            pMutex.lock();
            for (int y = y1; y < y2; ++y)
            {
                for (int x = x1; x < x2; ++x)
                {
                    if (yardmap.size() <= i)
                    {
                        pMutex.unlock();
                        return;
                    }
                    if(yardmap[i]=='G' || yardmap[i]=='o' || yardmap[i]=='w' || yardmap[i]=='f' || (yardmap[i]=='c' && !open) || (yardmap[i]=='C' && !open) || (yardmap[i]=='O' && open))
                        map_data[y][x].unit_idx=c;
                    ++i;
                }
                i += dw;
            }
            pMutex.unlock();
        }
    }


    void MAP::air_rect( int x1, int y1, int w, int h, short c, bool remove)
    {
        if (remove)
        {
            int y2=y1+h;
            int x2=x1+w;
            if(y1<0)	y1=0;
            if(y2>bloc_h_db-1)	y2=bloc_h_db-1;
            if(x1<0)	x1=0;
            if(x2>bloc_w_db-1)	x2=bloc_w_db-1;
            if(y2<=y1 || x2<=x1)	return;
            pMutex.lock();
            for(int y=y1;y<y2;y++)
                for(int x=x1;x<x2;x++)
                    map_data[y][x].air_idx.remove(c);
            pMutex.unlock();
        }
        else
        {
            int y2=y1+h;
            int x2=x1+w;
            if(y1<0)	y1=0;
            if(y2>bloc_h_db-1)	y2=bloc_h_db-1;
            if(x1<0)	x1=0;
            if(x2>bloc_w_db-1)	x2=bloc_w_db-1;
            if(y2<=y1 || x2<=x1)
                return;
            pMutex.lock();
            for(int y=y1;y<y2;y++)
                for(int x=x1;x<x2;x++)
                    map_data[y][x].air_idx.push(c);
            pMutex.unlock();
        }
    }

    bool MAP::check_rect(int x1,int y1,int w,int h,int c)
    {
        int y2=y1+h;
        int x2=x1+w;
        if(y1<0)	y1=0;
        if(y2>bloc_h_db-1)	y2=bloc_h_db-1;
        if(x1<0)	x1=0;
        if(x2>bloc_w_db-1)	x2=bloc_w_db-1;
        if(y2<=y1 || x2<=x1)	return false;
        for(int y=y1;y<y2;y++)
            for(int x=x1;x<x2;x++)
                if(map_data[y][x].unit_idx!=c && map_data[y][x].unit_idx!=-1)
                    return false;
        return true;
    }

    bool MAP::check_rect_discovered(int x1,int y1,int w,int h,int c)		// Check if the area has been fully discovered
    {
        int y2=(y1+h+1)>>1;
        int x2=(x1+w+1)>>1;
        x1>>=1;
        y1>>=1;
        if(y1<0)	y1=0;
        if(y2>bloc_h-1)	y2=bloc_h-1;
        if(x1<0)	x1=0;
        if(x2>bloc_w-1)	x2=bloc_w-1;
        if(y2<=y1 || x2<=x1)
            return false;
        for(int y=y1;y<y2;y++)
            for(int x=x1;x<x2;x++)
                if( !(SurfaceByte(view_map,x,y) & c) )
                    return false;
        return true;
    }


    float MAP::check_rect_dh(int x1,int y1,int w,int h)
    {
        int y2=y1+h;
        int x2=x1+w;
        if(y1<0)	y1=0;
        if(y2>bloc_h_db-1)	y2=bloc_h_db-1;
        if(x1<0)	x1=0;
        if(x2>bloc_w_db-1)	x2=bloc_w_db-1;
        float max_dh=0.0f;
        bool on_water=false;
        if(y2<=y1 || x2<=x1)	return max_dh;
        for (int y = y1; y < y2; ++y)
            for (int x = x1; x < x2; ++x)
            {
                if (map_data[y][x].dh>max_dh)
                    max_dh=map_data[y][x].dh;
                on_water|=map_data[y][x].underwater;
            }
        if(on_water)
            max_dh=-max_dh;
        return max_dh;
    }

    float MAP::check_max_depth(int x1,int y1,int w,int h)
    {
        int y2=y1+h;
        int x2=x1+w;
        if(y1<0)	y1=0;
        if(y2>bloc_h_db-1)	y2=bloc_h_db-1;
        if(x1<0)	x1=0;
        if(x2>bloc_w_db-1)	x2=bloc_w_db-1;
        float depth = -sealvl;
        if(y2<=y1 || x2<=x1)	return depth + sealvl;
        for(int y=y1;y<y2;y++)
            for(int x = x1; x < x2; ++x)
            {
                float d = -h_map[y][x];
                if(d>depth)
                    depth=d;
            }
        return depth + sealvl;
    }


    float MAP::check_min_depth(int x1,int y1,int w,int h)
    {
        int y2=y1+h;
        int x2=x1+w;
        if(y1<0)	y1=0;
        if(y2>bloc_h_db-1)	y2=bloc_h_db-1;
        if(x1<0)	x1=0;
        if(x2>bloc_w_db-1)	x2=bloc_w_db-1;
        float depth=255.0f-sealvl;
        if(y2<=y1 || x2<=x1)	return depth + sealvl;
        for(int y=y1;y<y2;y++)
            for(int x=x1;x<x2;x++)
            {
                float d = -h_map[y][x];
                if(d<depth)
                    depth=d;
            }
        return depth+sealvl;
    }


    bool MAP::check_vents(int x1,int y1,int w,int h,const String &yard_map)
    {
        if (yard_map.empty())
            return true;
        int y2=y1+h;
        int x2=x1+w;
        if(y1<0)	y1=0;
        if(y2>bloc_h_db-1)	y2=bloc_h_db-1;
        if(x1<0)	x1=0;
        if(x2>bloc_w_db-1)	x2=bloc_w_db-1;
        int dw = w - (x2-x1);
        int i = 0;
        bool ok = true;
        if(y2<=y1 || x2<=x1)
            return false;
        for (int y = y1; y < y2; ++y)
        {
            for (int x = x1; x < x2; ++x)
            {
                if (yard_map.size() <= i)	return ok;
                if (yard_map[i]=='G')
                {
                    ok = false;
                    if (map_data[y][x].stuff >= 0)
                    {
                        int feature_id = map_data[y][x].stuff;
                        if(feature_manager.feature[features.feature[feature_id].type].geothermal)
                            return true;
                    }
                }
                ++i;
            }
            i += dw;
        }
        return ok;
    }


    bool MAP::check_lava(int x1,int y1,int w,int h)
    {
        int y2=y1+h;
        int x2=x1+w;
        if (y1<0)
            y1 = 0;
        if (y2 > bloc_h - 1)
            y2 = bloc_h - 1;
        if (x1 < 0)
            x1 = 0;
        if (x2 > bloc_w - 1)
            x2 = bloc_w - 1;
        if (y2 <= y1 || x2 <= x1)
            return false;
        for (int y = y1; y < y2; ++y)
        {
            for (int x = x1; x < x2; ++x)
            {
                if(bloc[bmap[y][x]].lava)
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
                map_data[y][x].stuff = -1;
                map_data[y][x].unit_idx = -1;
            }
        }
    }

    void MAP::destroy()
    {
        /*------------- Experimental: code for new map format -----------------------*/
        /*	if(macro_bloc) {
            for(uint32 y=0;y<macro_h;y++)
            for(uint32 x=0;x<macro_w;x++)
            macro_bloc[y][x].destroy();
            delete[] macro_bloc[0];
            delete[] macro_bloc;
            macro_w=0;
            macro_h=0;
            }*/
        /*---------------------------------------------------------------------------*/

        if( view_map )		SDL_FreeSurface( view_map );
        if( sight_map )		SDL_FreeSurface( sight_map );
        if( radar_map )		SDL_FreeSurface( radar_map );
        if( sonar_map )		SDL_FreeSurface( sonar_map );

        detail_shader.destroy();
        shadow2_shader.destroy();
        gfx->destroy_texture( details_tex );

        if(low_vtx)			delete[] low_vtx;
        if(low_vtx_flat)	delete[] low_vtx_flat;
        if(low_tcoord)		delete[] low_tcoord;
        if(low_col)			delete[] low_col;
        if(low_index)		delete[] low_index;
        if(low_tex)			glDeleteTextures(1,&low_tex);

        ota_data.destroy();
        gfx->destroy_texture( lava_map );
        if(path && bloc_w && bloc_h) {
            delete[] path[0];
            delete[] path;
        }
        if(view && bloc_w && bloc_h) {
            delete[] view[0];
            delete[] view;
        }
        if(map_data && bloc_w && bloc_h) {
            delete[] map_data[0];
            delete[] map_data;
        }
        if(ph_map && bloc_w && bloc_h) {
            delete[] ph_map[0];		// la carte est allouée d'un seul bloc
            delete[] ph_map;
        }
        if(ph_map_2 && bloc_w && bloc_h) {
            delete[] ph_map_2[0];		// la carte est allouée d'un seul bloc
            delete[] ph_map_2;
        }
        if(h_map && bloc_w && bloc_h) {
            delete[] h_map[0];		// la carte est allouée d'un seul bloc
            delete[] h_map;
        }
        if(bmap && bloc_w && bloc_h) {
            delete[] bmap[0];		// la carte est allouée d'un seul bloc
            delete[] bmap;
        }
        if(ntex>0) {
            for(int i=0;i<ntex;i++)
                gfx->destroy_texture( tex[i] );
            delete[] tex;
        }
        if(lvl) {
            for(int i=0;i<bloc_h*bloc_w;i++)
                delete[] lvl[i];
            delete[] lvl;
        }
        if(bloc && nbbloc>0) {
            for(int i=0;i<nbbloc;i++) {
                bloc[i].point=NULL;
                bloc[i].destroy();
            }
            delete[] bloc;
        }
        if(mini) {
            gfx->destroy_texture( glmini );
            SDL_FreeSurface(mini);
        }
        init();
        detail_shader.destroy();		// Because init will reload it

        the_map = NULL;
    }

    void MAP::clear_FOW( sint8 FOW_flags )
    {
        if (FOW_flags < 0)	FOW_flags = fog_of_war;
        fog_of_war = FOW_flags;

        if (fog_of_war & FOW_BLACK)
            memset( view_map->pixels, 0, view_map->w * view_map->h );
        else
            memset( view_map->pixels, 0xFF, view_map->w * view_map->h );
        if (fog_of_war & FOW_GREY)
            memset( sight_map->pixels, 0, sight_map->w * sight_map->h );
        else
            memset( sight_map->pixels, 0xFF, sight_map->w * sight_map->h );

        if (fog_of_war == FOW_DISABLED)
        {
            memset( radar_map->pixels, 0xFF, radar_map->w * radar_map->h );
            memset( sonar_map->pixels, 0xFF, sonar_map->w * sonar_map->h );
        }
    }

    void MAP::load_details_texture( const String &filename )
    {
        SDL_Surface *tex = gfx->load_image(filename);
        if (tex)
        {
            uint32 average = 0;
            for( int y = 0 ; y < tex->h ; y++ )
                for( int x = 0 ; x < tex->w ; x++ )
                    average += SurfaceByte(tex, x<<2, y) + SurfaceByte(tex,(x<<2)+1,y) + SurfaceByte(tex,(x<<2)+2,y);
            average /= tex->w * tex->h * 3;
            if( average == 0 )	average = 1;
            color_factor = 255.0f / average;
            details_tex = gfx->make_texture( tex, FILTER_TRILINEAR, false );
            SDL_FreeSurface( tex );
        }
        else
        {
            details_tex = 0;
            color_factor = 1.0f;
        }
    }

    void MAP_OTA::load( String filename )
    {
        uint32 ota_file_size = 0;
        byte *data = HPIManager->PullFromHPI( filename, &ota_file_size );
        if( data ) {
            load( (char*)data, ota_file_size );
            delete[] data;
        }
    }

    void MAP_OTA::load(char *data,int ota_size)
    {
        destroy();

        TDFParser parser;
        parser.loadFromMemory("OTA",data,ota_size,false,true,false);

        missionname = parser.pullAsString("GlobalHeader.missionname");
        planet = parser.pullAsString("GlobalHeader.planet");
        glamour = parser.pullAsString("GlobalHeader.glamour");
        missiondescription = parser.pullAsString("GlobalHeader.missiondescription");
        tidalstrength = parser.pullAsInt("GlobalHeader.tidalstrength");
        solarstrength = parser.pullAsInt("GlobalHeader.solarstrength");
        lavaworld = parser.pullAsBool("GlobalHeader.lavaworld");
        killmul = parser.pullAsInt("GlobalHeader.killmul", 50);
        minwindspeed = parser.pullAsInt("GlobalHeader.minwindspeed");
        maxwindspeed = parser.pullAsInt("GlobalHeader.maxwindspeed");
        gravity = parser.pullAsInt("GlobalHeader.gravity") * 0.1f;
        numplayers = parser.pullAsString("GlobalHeader.numplayers");
        map_size = parser.pullAsString("GlobalHeader.size");
        SurfaceMetal = parser.pullAsInt("GlobalHeader.Schema 0.SurfaceMetal");
        MohoMetal = parser.pullAsInt("GlobalHeader.Schema 0.MohoMetal");
        for(int s = 0 ; parser.exists( format("globalheader.schema 0.specials.special%d", s) ) ; s++)
        {
            String key = format("GlobalHeader.Schema 0.specials.special%d.", s);
            String specialWhat = parser.pullAsString(key + "specialwhat");
            if (StartsWith( specialWhat.toLower(), "startpos"))
            {
                int index = String(specialWhat.substr(8, specialWhat.size() - 8)).toInt32() - 1;
                startX[index] = parser.pullAsInt(key + "xpos");
                startZ[index] = parser.pullAsInt(key + "zpos");
            }
        }
        waterdoesdamage = parser.pullAsBool("GlobalHeader.waterdoesdamage");
        waterdamage = parser.pullAsInt("GlobalHeader.waterdamage");
        network = StartsWith( parser.pullAsString("GlobalHeader.Schema 0.type").toLower(), "network");

        if(waterdamage==0)
            waterdoesdamage=false;
    }

    void MAP::draw_mini(int x1,int y1,int w,int h, Camera* cam, byte player_mask)			// Draw the mini-map
    {
        if (!mini)
            return;		// Check if it exists

        gfx->set_color(0xFFFFFFFF);

        int rw = w * mini_w / 252;
        int rh = h * mini_h / 252;
        x1 += (w - rw) >> 1;
        y1 += (h - rh) >> 1;
        float lw = mini_w / 252.0f;
        float lh = mini_h / 252.0f;
        gfx->drawtexture(glmini, x1, y1, x1 + rw, y1 + rh, 0.0f, 0.0f, lw, lh);

        if (rh == 0 || rw == 0) return;

        if (fog_of_war != FOW_DISABLED)
        {
            glEnable( GL_BLEND );
            glBlendFunc( GL_ZERO, GL_SRC_COLOR );			// Special blending function
            glDisable( GL_TEXTURE_2D );
            glBegin( GL_LINES );

            int MY = 0;
            int DY = 0x10000 * ( bloc_h_db - 2 ) / rh;
            int DX = 0x10000 * ( bloc_w_db - 2 ) / rw;

            gfx->lock();

            for (int y = 0; y < rh; ++y)
            {
                int my = MY >> 17;
                MY += DY;
                int old_col = -1;
                int old_x = -1;
                int MX = 0;
                for (int x = 0 ; x < rw; ++x)
                {
                    int mx = MX >> 17;
                    MX += DX;
                    if(!(SurfaceByte(view_map,mx,my) & player_mask))
                    {
                        if( old_col != 0 )
                        {
                            if( old_x != -1 )
                            {
                                glVertex2i( x1+old_x, y1+y );
                                glVertex2i( x1+x, y1+y );
                            }
                            glColor3ub( 0, 0, 0 );
                            old_col = 0;
                            old_x = x;
                        }
                    }
                    else
                    {
                        if(!(SurfaceByte(sight_map,mx,my) & player_mask))
                        {
                            if( old_col != 1 )
                            {
                                if( old_x != -1 )
                                {
                                    glVertex2i( x1+old_x, y1+y );
                                    glVertex2i( x1+x, y1+y );
                                }
                                glColor3ub( 0x7F, 0x7F, 0x7F );
                                old_x = x;
                                old_col = 1;
                            }
                        }
                        else
                        {
                            if( old_col != 2 )
                            {
                                if( old_x != -1 )
                                {
                                    glVertex2i( x1+old_x, y1+y );
                                    glVertex2i( x1+x, y1+y );
                                }
                                old_x = -1;
                                old_col = 2;
                            }
                        }
                    }
                }
                if( old_x != -1)
                {
                    glVertex2i( x1+old_x, y1+y );
                    glVertex2i( x1+rw, y1+y );
                }
            }

            glEnd();
            glDisable(GL_BLEND);

            gfx->unlock();
        }

        if(!cam)
            return;

        Vector3D P;
        Vector3D A, B, C, D;
        Vector3D PA, PB, PC, PD;

        if (lp_CONFIG->ortho_camera)
        {
            A = B = C = D = cam->dir;
            PA = cam->pos + cam->zoomFactor * ( -gfx->SCREEN_W_HALF * cam->side - gfx->SCREEN_H_HALF * cam->up );
            PB = cam->pos + cam->zoomFactor * ( gfx->SCREEN_W_HALF * cam->side - gfx->SCREEN_H_HALF * cam->up );
            PC = cam->pos + cam->zoomFactor * ( gfx->SCREEN_W_HALF * cam->side + gfx->SCREEN_H_HALF * cam->up );
            PD = cam->pos + cam->zoomFactor * ( -gfx->SCREEN_W_HALF * cam->side + gfx->SCREEN_H_HALF * cam->up );
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
        float cx[4*nmax+4],cy[4*nmax+4];
        if(A.y<0.0f) {
            P = PA + PA.y / fabsf(A.y)*A;	cx[0]=P.x;	cy[0]=P.z; }
        else {
            P = PA + 10000.0f*A;	cx[0]=P.x;	cy[0]=P.z; }
        if(B.y<0.0f) {
            P = PB + PB.y / fabsf(B.y)*B;	cx[1]=P.x;	cy[1]=P.z; }
        else {
            P = PB + 10000.0f*B;	cx[1]=P.x;	cy[1]=P.z; }
        if(C.y<0.0f) {
            P = PC + PC.y / fabsf(C.y)*C;	cx[2]=P.x;	cy[2]=P.z; }
        else {
            P = PC + 10000.0f*C;	cx[2]=P.x;	cy[2]=P.z; }
        if(D.y<0.0f) {
            P = PD + PD.y / fabsf(D.y)*D;	cx[3]=P.x;	cy[3]=P.z; }
        else {
            P = PD + 10000.0f*D;	cx[3]=P.x;	cy[3]=P.z; }

        for (byte i = 0; i < 4; ++i)
        {
            cx[i] = (cx[i] + 0.5f * map_w) * rw / map_w;
            cy[i] = (cy[i] + 0.5f * map_h) * rh / map_h;
        }
        for (byte i = 0; i < 4; ++i)
        {
            for (int e = 0; e < nmax; ++e)
            {
                cx[i * nmax + e + 4] = (cx[i] * (nmax - e) + cx[(i + 1) % 4] * (e + 1)) / (nmax + 1);
                cy[i * nmax + e + 4] = (cy[i] * (nmax - e) + cy[(i + 1) % 4] * (e + 1)) / (nmax + 1);
            }
        }
        for (int i = 0; i < 4 + (nmax << 2); ++i)
        {
            if (cx[i] < 0.0f)
                cx[i] = 0.0f;
            else
                if(cx[i] > rw)
                    cx[i] = rw;
            if (cy[i] < 0.0f)
                cy[i] = 0.0f;
            else
                if (cy[i] > rh)
                    cy[i] = rh;
        }

        glDisable(GL_TEXTURE_2D);
        glColor3ub(0xE5,0xE5,0x66);
        glBegin(GL_LINE_LOOP);
        for (byte i = 0; i < 4; ++i)
        {
            glVertex2f(cx[i] + x1,  cy[i] + y1);
            for (int e = 0; e < nmax; ++e)
                glVertex2f(x1 + cx[i * nmax + e + 4],  y1 + cy[i * nmax + e + 4]);
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
        if( jamming )
        {
            if( rd_j > 0 )
                circlefill( radar_map, px, py, rd_j, 0 );
            if( sn_j > 0 )
                circlefill( sonar_map, px, py, sn_j, 0 );
        }
        else
        {
            byte mask = 1 << player_id;
            uint32 mask32 = 0x01010101 << player_id;
            int r2 = r * r;
            int rd2 = rd * rd;
            int sn2 = sn * sn;
            // Update detector maps
            if (sn > 0)
                for (int y = 0; y <= sn; ++y) // Update sonar data
                {
                    int x=(int)(0.5f+sqrtf((float)(sn2-y*y)));
                    int ry=py-y;
                    if (ry >= 0 && ry < sonar_map->h)
                    {
                        int rx=px-x;
                        int lx=px+x;
                        if(rx<0)	rx=0;
                        if(lx>=sonar_map->w)	lx=sonar_map->w-1;
                        for(; (rx & 3) && rx < view_map->w ; rx++ )
                            SurfaceByte(sonar_map,rx,ry) |= mask;
                        rx >>= 2;
                        int lx2 = lx >> 2;
                        for(;rx<lx2;rx++)
                            SurfaceInt(sonar_map,rx,ry) |= mask32;
                        rx <<= 2;
                        for(; rx <= lx ; rx++ )
                            SurfaceByte(sonar_map,rx,ry) |= mask;
                    }
                    if(y != 0)
                    {
                        ry = py + y;
                        if (ry >= 0 && ry < sonar_map->h)
                        {
                            int rx = px - x;
                            int lx = px + x;
                            if(rx < 0)
                                rx = 0;
                            if (lx >= sonar_map->w)
                                lx = sonar_map->w - 1;
                            for(; (rx & 3) && rx < view_map->w ; ++rx)
                                SurfaceByte(sonar_map,rx,ry) |= mask;
                            rx >>= 2;
                            int lx2 = lx >> 2;
                            for(;rx<lx2;rx++)
                                SurfaceInt(sonar_map,rx,ry) |= mask32;
                            rx <<= 2;
                            for(; rx <= lx ; rx++ )
                                SurfaceByte(sonar_map,rx,ry) |= mask;
                        }
                    }
                }
            if( rd > 0 )
                for (int y = 0; y <= rd; ++y) // Update radar data
                {
                    int x=(int)(0.5f+sqrtf((float)(rd2-y*y)));
                    int ry=py-y;
                    if (ry >= 0 && ry < radar_map->h)
                    {
                        int rx=px-x;
                        int lx=px+x;
                        if(rx<0)	rx=0;
                        if(lx>=radar_map->w)	lx=radar_map->w-1;
                        for(; (rx & 3) && rx < view_map->w ; rx++ )
                            SurfaceByte(radar_map,rx,ry) |= mask;
                        rx >>= 2;
                        int lx2 = lx >> 2;
                        for(;rx<lx2;rx++)
                            SurfaceInt(radar_map,rx,ry) |= mask32;
                        rx <<= 2;
                        for(; rx <= lx ; rx++ )
                            SurfaceByte(radar_map,rx,ry) |= mask;
                    }
                    if (y != 0)
                    {
                        ry = py + y;
                        if (ry >= 0 && ry < radar_map->h)
                        {
                            int rx=px-x;
                            int lx=px+x;
                            if(rx<0)	rx=0;
                            if(lx>=radar_map->w)	lx=radar_map->w-1;
                            for(; (rx & 3) && rx < view_map->w ; rx++ )
                                SurfaceByte(radar_map,rx,ry) |= mask;
                            rx >>= 2;
                            int lx2 = lx >> 2;
                            for(;rx<lx2;rx++)
                                SurfaceInt(radar_map,rx,ry) |= mask32;
                            rx <<= 2;
                            for(; rx <= lx ; rx++ )
                                SurfaceByte(radar_map,rx,ry) |= mask;
                        }
                    }
                }
            if (fog_of_war & FOW_GREY)
                for(int y = 0; y <= r; ++y) // Update view data
                {
                    int x=(int)(0.5f+sqrtf((float)(r2-y*y)));
                    int ry=py-y;
                    if (ry >= 0 && ry < sight_map->h)
                    {
                        int rx=px-x;
                        int lx=px+x;
                        if(rx<0)	rx=0;
                        if(lx>=sight_map->w)	lx=sight_map->w-1;
                        for(; (rx & 3) && rx < view_map->w ; rx++ )
                            SurfaceByte(sight_map,rx,ry) |= mask;
                        rx >>= 2;
                        int lx2 = lx >> 2;
                        for(;rx<lx2;rx++)
                            SurfaceInt(sight_map,rx,ry) |= mask32;
                        rx <<= 2;
                        for(; rx <= lx ; rx++ )
                            SurfaceByte(sight_map,rx,ry) |= mask;
                    }
                    if (y != 0)
                    {
                        ry = py + y;
                        if (ry >= 0 && ry<sight_map->h)
                        {
                            int rx=px-x;
                            int lx=px+x;
                            if(rx<0)	rx=0;
                            if(lx>=sight_map->w)	lx=sight_map->w-1;
                            for(; (rx & 3) && rx < view_map->w ; rx++ )
                                SurfaceByte(sight_map,rx,ry) |= mask;
                            rx >>= 2;
                            int lx2 = lx >> 2;
                            for(;rx<lx2;rx++)
                                SurfaceInt(sight_map,rx,ry) |= mask32;
                            rx <<= 2;
                            for(; rx <= lx ; rx++ )
                                SurfaceByte(sight_map,rx,ry) |= mask;
                        }
                    }
                }
            if (black && (fog_of_war & FOW_BLACK))
                for (int y = 0; y <= r; ++y) // Update view data
                {
                    int x=(int)(0.5f+sqrtf((float)(r2-y*y)));
                    int ry=py-y;
                    if (ry >= 0 && ry < view_map->h)
                    {
                        int rx=px-x;
                        int lx=px+x;
                        if(rx<0)	rx=0;
                        if(lx>=view_map->w)	lx=view_map->w-1;
                        for(; (rx & 3) && rx < view_map->w ; rx++ )
                            SurfaceByte(view_map,rx,ry) |= mask;
                        rx >>= 2;
                        int lx2 = lx >> 2;
                        for(;rx<lx2;rx++)
                            SurfaceInt(view_map,rx,ry) |= mask32;
                        rx <<= 2;
                        for(; rx <= lx ; rx++ )
                            SurfaceByte(view_map,rx,ry) |= mask;
                    }
                    if (y != 0)
                    {
                        ry = py + y;
                        if (ry >= 0 && ry < view_map->h)
                        {
                            int rx=px-x;
                            int lx=px+x;
                            if(rx<0)	rx=0;
                            if(lx>=view_map->w)	lx=view_map->w-1;
                            for(; (rx & 3) && rx < view_map->w ; rx++ )
                                SurfaceByte(view_map,rx,ry) |= mask;
                            rx >>= 2;
                            int lx2 = lx >> 2;
                            for(;rx<lx2;rx++)
                                SurfaceInt(view_map,rx,ry) |= mask32;
                            rx <<= 2;
                            for(; rx <= lx ; rx++ )
                                SurfaceByte(view_map,rx,ry) |= mask;
                        }
                    }
                }
        }
        gfx->unlock();
    }

    void MAP::check_unit_visibility(int x, int y)
    {
        if(y<0 || y>bloc_h_db-1 || x<0 || x>bloc_w_db-1)	return;
        int idx = map_data[y][x].unit_idx;
        if(idx >= 0 && !units.unit[idx].visibility_checked)
        {
            units.visible_unit.push_back(idx);
            units.unit[idx].visibility_checked = true;
        }
        pMutex.lock();
        IDX_LIST_NODE *cur = map_data[y][x].air_idx.head;
        while(cur)
        {
            idx = cur->idx;
            if(idx >= 0 && !units.unit[idx].visibility_checked)
            {
                units.visible_unit.push_back(idx);
                units.unit[idx].visibility_checked = true;
            }
            cur = cur->next;
        }
        pMutex.unlock();
    }

    std::vector<Vector3D> MAP::get_visible_volume()
    {
        std::vector<Vector3D>  volume = Camera::inGame->getFrustum();
        for(int i = 4 ; i < 8 ; i++)
        {
            Vector3D dir = volume[i] - volume[i-4];
            float dist_max = dir.norm();
            dir = 1.0f / dist_max * dir;;
            if (dir.y > 0.0f)       // Heading up
            {
                volume[i] += (512.0f * H_DIV - volume[i].y) * dir;
                continue;
            }
            Vector3D map_hit = hit( volume[i-4], dir, false, dist_max, true) + 30.0f * dir;
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

        bool low_def_view = cam->rpos.y>gfx->low_def_limit;		// Low detail map for mega zoom

        if (check_visibility && !FLAT)
            units.visible_unit.clear();

        if (low_def_view && check_visibility && !FLAT)
        {
            for(int i = 0 ; i < units.index_list_size ; ++i)
                units.visible_unit.push_back( units.idx_list[i] );
        }

        gfx->lock();

        if(FLAT)
            glTranslatef(0.0f,0.0f,sea_dec);

        int i,x,y;
        glDisable(GL_CULL_FACE);
        glDisable(GL_LIGHTING);
        if(!FLAT)
        {
            if(ntex>0)
                gfx->ReInitAllTex( true );
            else
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
        Vector3D P;
        Vector3D A, B, C, D;
        Vector3D PA, PB, PC, PD;

        if (lp_CONFIG->ortho_camera)
        {
            A = B = C = D = cam->dir;
            PA = cam->pos + cam->zoomFactor * ( -gfx->SCREEN_W_HALF * cam->side - gfx->SCREEN_H_HALF * cam->up );
            PB = cam->pos + cam->zoomFactor * ( gfx->SCREEN_W_HALF * cam->side - gfx->SCREEN_H_HALF * cam->up );
            PC = cam->pos + cam->zoomFactor * ( gfx->SCREEN_W_HALF * cam->side + gfx->SCREEN_H_HALF * cam->up );
            PD = cam->pos + cam->zoomFactor * ( -gfx->SCREEN_W_HALF * cam->side + gfx->SCREEN_H_HALF * cam->up );
        }
        else
        {
            A = cam->dir + cam->widthFactor * (-cam->side) - 0.75f * cam->up;
            B = cam->dir + cam->widthFactor * (cam->side)  - 0.75f * cam->up;
            C = cam->dir + cam->widthFactor * (cam->side)  + 0.75f * cam->up;
            D = cam->dir + cam->widthFactor * (-cam->side) + 0.75f * cam->up;
            PA = PB = PC = PD = cam->pos;
        }
        float cx[4],cy[4];
        if(A.y<0.0f) {
            P = PA + PA.y / fabsf(A.y)*A;	cx[0]=P.x;	cy[0]=P.z; }
        else {
            P = PA + 10000.0f*A;	cx[0]=P.x;	cy[0]=P.z; }
        if(B.y<0.0f) {
            P = PB + PB.y / fabsf(B.y)*B;	cx[1]=P.x;	cy[1]=P.z; }
        else {
            P = PB + 10000.0f*B;	cx[1]=P.x;	cy[1]=P.z; }
        if(C.y<0.0f) {
            P = PC + PC.y / fabsf(C.y)*C;	cx[2]=P.x;	cy[2]=P.z; }
        else {
            P = PC + 10000.0f*C;	cx[2]=P.x;	cy[2]=P.z; }
        if(D.y<0.0f) {
            P = PD + PD.y / fabsf(D.y)*D;	cx[3]=P.x;	cy[3]=P.z; }
        else {
            P = PD + 10000.0f*D;	cx[3]=P.x;	cy[3]=P.z; }

        int minx=bloc_w<<1,maxx=0;
        int miny=bloc_h<<1,maxy=0;
        for(i=0;i<4;i++)
        {
            cx[i]=(cx[i]+0.5f*map_w)/16.0f;
            cy[i]=(cy[i]+0.5f*map_h)/16.0f;
            if(cx[i]<minx) minx=(int)cx[i];
            if(cx[i]>maxx) maxx=(int)cx[i];
            if(cy[i]<miny) miny=(int)cy[i];
            if(cy[i]>maxy) maxy=(int)cy[i];
        }
        int y1=bloc_h,y2=0,x1=bloc_w,x2=0,mx,my;
        float limit=cam->zfar*sqrtf(2.0f);
        x1=(int)((cam->pos.x+0.5f*map_w-limit)/16.0f);
        y1=(int)((cam->pos.z+0.5f*map_h-limit)/16.0f);
        x2=(int)((cam->pos.x+0.5f*map_w+limit)/16.0f);
        y2=(int)((cam->pos.z+0.5f*map_h+limit)/16.0f);
        mx=(int)((cam->pos.x+0.5f*map_w)/16.0f);
        my=(int)((cam->pos.z+0.5f*map_h)/16.0f);
        if(x1<minx)
            x1=minx;
        if(x2>maxx)
            x2=maxx;
        if(y1<miny)
            y1=miny;
        if(y2>maxy)
            y2=maxy;
        if(x1>mx) x1=mx;
        if(y1>my) y1=my;
        if(x2<mx) x2=mx;
        if(y2<my) y2=my;

        if((abs(my-y2)<<4)>cam->zfar+64.0f)
            y2 = my>y2 ? my - (int)(cam->zfar/16.0f)-4 : my + 4 + (int)(cam->zfar/16.0f);
        if((abs(my-y1)<<4)>cam->zfar+64.0f)
            y1 = my>y1 ? my - (int)(cam->zfar/16.0f)-4 : my + 4 + (int)(cam->zfar/16.0f);
        if((abs(mx-x2)<<4)>cam->zfar+64.0f)
            x2 = mx>x2 ? mx - (int)(cam->zfar/16.0f)-4 : mx + 4 + (int)(cam->zfar/16.0f);
        if((abs(mx-x1)<<4)>cam->zfar+64.0f)
            x1 = mx>x1 ? mx - (int)(cam->zfar/16.0f)-4 : mx + 4 + (int)(cam->zfar/16.0f);

        if(y1<0) y1=0;
        if(y2<0) y2=0;
        if(y1>=bloc_h) y1=bloc_h-1;
        if(y2>=bloc_h) y2=bloc_h-1;
        if(x1<0) x1=0;
        if(x2<0) x2=0;
        if(x1>=bloc_w) x1=bloc_w-1;
        if(x2>=bloc_w) x2=bloc_w-1;

        y1 -= 3;
        if (y1 < 0)
            y1 = 0;

        A = (cam->dir + 0.75f * cam->up - cam->widthFactor * cam->side);
        A.unit();

        float ref = sq( A%cam->dir );
        // Here we use an approximation of the right formula, assuming M should remain small compared to the rest (remember we use squared values ...)
        // the right formula that gives the ref0 value we want is : cosf( a ) - r / D * sqrtf( 1 / tan( a ) )
        // where 2 * a is the camera aperture angle, 2 * r the diameter of a map bloc and D the distance from the camera to the bloc
        // with ref = cosf( a )² we compute: ref0 ~ ref - 2 * r / D * sqrtf( 1 / tan( a ) ) = ref - M / D
        float M = H_DIV * 512.0f * sqrtf( (A%cam->dir) / sqrtf( 1.0f - sq(A%cam->dir) ) );    // H_DIV * 512 because it's twice H_DIV * 256, maximum height of a map bloc
        float dhm=0.5f*map_h;
        float dwm=0.5f*map_w;

        if(!FLAT)
            glColor4f(1.0f,1.0f,1.0f,1.0f);

        Vector3D flat[9];
        if (FLAT)
        {
            flat[0].x=0.0f;		flat[0].y=niv+cosf(t)*0.5f;			flat[0].z=0.0f;
            flat[1].x=8.0f;		flat[1].y=niv+cosf(t+1.0f)*0.5f;		flat[1].z=0.0f;
            flat[2].x=16.0f;	flat[2].y=flat[0].y;				flat[2].z=0.0f;
            flat[3].x=0.0f;		flat[3].y=niv+cosf(t+1.5f)*0.5f;		flat[3].z=8.0f;
            flat[4].x=8.0f;		flat[4].y=niv+cosf(t+2.5f)*0.5f;		flat[4].z=8.0f;
            flat[5].x=16.0f;	flat[5].y=flat[3].y;				flat[5].z=8.0f;
            flat[6].x=0.0f;		flat[6].y=flat[0].y;				flat[6].z=16.0f;
            flat[7].x=8.0f;		flat[7].y=flat[1].y;				flat[7].z=16.0f;
            flat[8].x=16.0f;	flat[8].y=flat[0].y;				flat[8].z=16.0f;
        }

        bool enable_details = !cam->mirror && (lp_CONFIG->detail_tex || lp_CONFIG->shadow_quality >= 2);

        if (ntex > 0 && !depth_only)
        {
            glActiveTextureARB(GL_TEXTURE0_ARB);
            glEnable(GL_TEXTURE_2D);
            glClientActiveTextureARB(GL_TEXTURE0_ARB);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);

            if (lp_CONFIG->detail_tex && !FLAT && enable_details)
            {
                glClientActiveTextureARB(GL_TEXTURE1_ARB);
                glActiveTextureARB(GL_TEXTURE1_ARB );
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, details_tex );
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
            glTranslatef(cosf(t),0.0f,sinf(t));
        GLuint old_tex=bloc[0].tex;
        if (!depth_only)
            glBindTexture(GL_TEXTURE_2D,old_tex);
        if (!FLAT && check_visibility)
        {
            for(y = oy1; y <= oy2; ++y)
                memset(view[y] + ox1, 0, ox2 - ox1 + 1);
            features.min_idx = features.nb_features - 1;
            features.max_idx = 0;
            features.list_size = 0;
            ox1=x1;	ox2=x2;
            oy1=y1;	oy2=y2;
        }
        else
            if (!check_visibility)
            {
                x1=ox1;	x2=ox2;
                y1=oy1;	y2=oy2;
                for(int i=0;i<features.list_size;i++)
                    features.feature[features.list[i]].draw=true;
            }
        int lavaprob = (int)(1000 * dt);
        Vector3D buf_p[4500]; // Tampon qui accumule les blocs pour les dessiner en chaîne
        float	buf_t[9000];
        uint8	buf_c[18000];
        short	buf_size=0;				// in blocs
        uint16	index_size=0;
        bool	was_flat=false;
        glDisableClientState(GL_NORMAL_ARRAY);		// we don't need normal data
        glEnableClientState(GL_VERTEX_ARRAY);		// vertex coordinates
        glEnableClientState(GL_COLOR_ARRAY);		// Colors(for fog of war)
        glColorPointer(4,GL_UNSIGNED_BYTE,0,buf_c);
        glVertexPointer( 3, GL_FLOAT, 0, buf_p);

        if (!FLAT && enable_details)
        {
            switch(lp_CONFIG->shadow_quality)
            {
            case 2:
                shadow2_shader.on();
                shadow2_shader.setvar1f( "coef", color_factor );
                shadow2_shader.setvar1i( "details", 1 );
                shadow2_shader.setvar1i( "shadowMap", 7 );
                shadow2_shader.setmat4f( "light_Projection", gfx->shadowMapProjectionMatrix);
                break;
            default:
                detail_shader.on();
                detail_shader.setvar1f( "coef", color_factor );
                detail_shader.setvar1i( "details", 1 );
            };
        }

        glClientActiveTextureARB(GL_TEXTURE0_ARB );
        glTexCoordPointer(2, GL_FLOAT, 0, buf_t);

        int	ox=x1;

        if(low_def_view)							// draw the low detail map
        {
            detail_shader.off();
            glActiveTextureARB(GL_TEXTURE1_ARB );
            glDisable(GL_TEXTURE_2D);
            glActiveTextureARB(GL_TEXTURE0_ARB );

            i=0;
            for (y = 0; y <= low_h; ++y)
            {
                int Y = y * (bloc_h_db - 2) / low_h;
                for (x = 0; x <= low_w; ++x)
                {
                    int X = x * (bloc_w_db - 2) / low_w;
                    int Z;
                    Z=Y+get_zdec_notest(X,Y);					if(Z>=bloc_h_db-1)	Z=bloc_h_db-2;
                    if (!(SurfaceByte(view_map,X>>1,Z>>1) & player_mask))	low_col[i<<2]=low_col[(i<<2)+1]=low_col[(i<<2)+2]=low_col[(i<<2)+3]=0;
                    else
                    {
                        low_col[(i<<2)+3] = 255;
                        if (!(SurfaceByte(sight_map,X>>1,Z>>1) & player_mask))
                            low_col[i << 2] = low_col[(i << 2) + 1] = low_col[(i << 2) + 2] = 127;
                        else
                            low_col[i << 2] = low_col[(i << 2) + 1] = low_col[(i << 2) + 2] = 255;
                    }
                    ++i;
                }
            }
            glEnableClientState(GL_COLOR_ARRAY);
            glColorPointer(4,GL_UNSIGNED_BYTE,0,low_col);
            if (FLAT)
            {
                glTranslatef(0.0f,niv,0.0f);
                glVertexPointer( 3, GL_FLOAT, 0, low_vtx_flat);
            }
            else
                glVertexPointer( 3, GL_FLOAT, 0, low_vtx);
            glTexCoordPointer(2, GL_FLOAT, 0, low_tcoord);
            glBindTexture(GL_TEXTURE_2D,low_tex);
            glDrawRangeElements(GL_TRIANGLE_STRIP, 0, (low_w+1)*(low_h+1)-1, low_nb_idx,GL_UNSIGNED_INT,low_index);		// draw this map
        }

        if (low_def_view)
        {
            memset(view[0], 1, bloc_w * bloc_h);
            ox1 = 0;
            ox2 = bloc_w - 1;
            oy1 = 0;
            oy2 = bloc_h - 1;
        }

        Vector3D T;
        Vector3D V;
        if (!low_def_view)
        {
            for (y = y1; y <= y2; ++y) // Balaye les blocs susceptibles d'être visibles pour dessiner ceux qui le sont
            {
                int pre_y = y<< 4;
                int Y = y << 1;
                int pre_y2 = y * bloc_w;
                T.x = -dwm;
                T.y = 0.0f;
                T.z = pre_y - dhm;
                buf_size = 0;
                ox = x1;
                bool was_clean = false;

                int rx1 = x1;
                int rx2 = x2;

                if (!FLAT && check_visibility)
                {
                    for (; rx1 <= x2 ; ++rx1)
                    {
                        int X = rx1<<1;
                        V.x = (rx1<<4) - dwm;
                        V.y = ph_map[Y|1][X|1];
                        V.z = pre_y - dhm + get_zdec_notest(X|1,Y|1);
                        V = V - cam->pos;
                        if(fabsf(V%cam->dir) > cam->zfar)
                        {
                            view[y][rx1] = 0;
                            continue;
                        }
                        float d = V.sq();
                        if(d > 16384.0f)
                            if(sq(V % cam->dir) < ref * d - M * sqrtf(d))
                            {
                                view[y][rx1] = 0;
                                continue;
                            }
                        break;
                    }
                    for (; rx2 >= rx1; --rx2)
                    {
                        int X = rx2<<1;
                        V.x = (rx2<<4) - dwm;
                        V.y = ph_map[Y|1][X|1];
                        V.z = pre_y - dhm + get_zdec_notest(X|1,Y|1);
                        V = V - cam->pos;
                        if(fabsf(V % cam->dir) > cam->zfar)
                        {
                            view[y][rx2] = 0;
                            continue;
                        }
                        float d = V.sq();
                        if(d > 16384.0f)
                            if(sq(V % cam->dir) < ref * d - M * sqrtf(d))
                            {
                                view[y][rx2] = 0;
                                continue;
                            }
                        break;
                    }
                }

                for (x = rx1; x <= rx2; ++x)
                {
                    int X = x<< 1;
                    if (!FLAT && check_visibility)
                    {
                        if (!(SurfaceByte(view_map,x,y) & player_mask))
                        {
                            if (water)
                            {
                                if (map_data[Y][X].underwater && map_data[Y|1][X].underwater && map_data[Y][X|1].underwater && map_data[Y|1][X|1].underwater)
                                    view[y][x] = 2;
                                else
                                    view[y][x] = 3;
                            }
                        }
                        else
                        {
                            if (!(SurfaceByte(sight_map,x,y) & player_mask))
                            {
                                if (map_data[Y][X].underwater || map_data[Y|1][X].underwater || map_data[Y][X|1].underwater || map_data[Y|1][X|1].underwater)
                                    view[y][x]=2;
                                else
                                    view[y][x]=3;
                            }
                            else
                                view[y][x]=1;
                            check_unit_visibility(X, Y);
                            check_unit_visibility(X, Y|1);
                            check_unit_visibility(X|1, Y);
                            check_unit_visibility(X|1, Y|1);

                            if (map_data[Y][X].stuff>=0 && map_data[Y][X].stuff<features.max_features) // Indique comme affichables les objets présents sur le bloc
                            {
                                if (features.feature[map_data[Y][X].stuff].type<0)
                                    map_data[Y][X].stuff=-1;
                                else
                                {
                                    features.feature[map_data[Y][X].stuff].draw=true;
                                    features.feature[map_data[Y][X].stuff].grey=(view[y][x]&2)==2;
                                    features.list[features.list_size++]=map_data[Y][X].stuff;
                                }
                            }
                            if (map_data[Y][X|1].stuff>=0 && map_data[Y][X|1].stuff<features.max_features)
                            {
                                if (features.feature[map_data[Y][X|1].stuff].type<0)
                                    map_data[Y][X|1].stuff=-1;
                                else
                                {
                                    features.feature[map_data[Y][X|1].stuff].draw=true;
                                    features.feature[map_data[Y][X|1].stuff].grey=(view[y][x]&2)==2;
                                    features.list[features.list_size++]=map_data[Y][X|1].stuff;
                                }
                            }
                            if (map_data[Y|1][X].stuff>=0 && map_data[Y|1][X].stuff<features.max_features)
                            {
                                if (features.feature[map_data[Y|1][X].stuff].type<0)
                                    map_data[Y|1][X].stuff=-1;
                                else
                                {
                                    features.feature[map_data[Y|1][X].stuff].draw=true;
                                    features.feature[map_data[Y|1][X].stuff].grey=(view[y][x]&2)==2;
                                    features.list[features.list_size++]=map_data[Y|1][X].stuff;
                                }
                            }
                            if (map_data[Y|1][X|1].stuff>=0 && map_data[Y|1][X|1].stuff<features.max_features)
                            {
                                if (features.feature[map_data[Y|1][X|1].stuff].type<0)
                                    map_data[Y|1][X|1].stuff=-1;
                                else
                                {
                                    features.feature[map_data[Y|1][X|1].stuff].draw=true;
                                    features.feature[map_data[Y|1][X|1].stuff].grey=(view[y][x]&2)==2;
                                    features.list[features.list_size++]=map_data[Y|1][X|1].stuff;
                                }
                            }
                        }
                    }
                    else
                    {
                        if (view[y][x] == 0)
                            continue;
                        if (view[y][x] == 2 && !draw_uw)
                            continue;		// Jump this if it is under water and don't have to be drawn
                        if (view[y][x] == 3)
                            view[y][x] = 2;
                        if (view[y][x]==2 && FLAT)
                            view[y][x]=0;
                        if (cam->mirror && map_data[Y][X].flat)
                            continue;
                    }
                    if (low_def_view)
                        continue;
                    // Si le joueur ne peut pas voir ce morceau, on ne le dessine pas en clair
                    T.x+=x<<4;
                    i=bmap[y][x];
                    if (FLAT)
                    {
                        bloc[i].point=lvl[pre_y2+x];
                        if (bloc[i].point==NULL || bloc[i].point[0].y<niv || bloc[i].point[1].y<niv || bloc[i].point[2].y<niv ||
                           bloc[i].point[3].y<niv || bloc[i].point[4].y<niv || bloc[i].point[5].y<niv ||
                           bloc[i].point[6].y<niv || bloc[i].point[7].y<niv || bloc[i].point[8].y<niv)
                            bloc[i].point=flat;
                        else
                        {
                            T.x-=x<<4;
                            continue;
                        }
                    }
                    else
                    {
                        if (check_visibility)
                        {
                            bool under_water = (h_map[Y|1][X|1] < sealvl && h_map[Y][X|1] < sealvl && h_map[Y|1][X] < sealvl && h_map[Y][X] < sealvl);

                            if ((bloc[i].lava || (under_water && ota_data.lavaworld) ) && !lp_CONFIG->pause
                                && (Math::RandFromTable()%1000000)<=lavaprob)		// Lava emiting code moved here because of lava effect using fragment program
                            {
                                Vector3D POS( (x<<4) - dwm + 8.0f, sealvl - 5.0f, pre_y - dhm + 8.0f );
                                V.x = ((Math::RandFromTable()%201)-100);
                                V.y = ((Math::RandFromTable()%51)+50);
                                V.z = ((Math::RandFromTable()%201)-100);
                                V.unit();
                                particle_engine.emit_lava(POS,V,1,10,(Math::RandFromTable()%1000)*0.01f+30.0f);
                            }
                            else
                            {
                                if( !map_data[ Y ][ X ].lava && water && !ota_data.lavaworld && !under_water && !lp_CONFIG->pause &&										// A wave
                                    (h_map[Y|1][X|1] < sealvl || h_map[Y][X|1] < sealvl || h_map[Y|1][X] < sealvl || h_map[Y][X] < sealvl) &&
                                    (h_map[Y|1][X|1] >= sealvl || h_map[Y|1][X] >= sealvl || h_map[Y][X|1] >= sealvl || h_map[Y][X] >= sealvl) &&
                                    (Math::RandFromTable()%4000)<=lavaprob &&
                                    (SurfaceByte(view_map,x,y) & player_mask) && lp_CONFIG->waves )
                                {
                                    Vector3D POS;
                                    POS.x = (x << 4) - dwm + 8.0f;
                                    POS.z = pre_y - dhm + 8.0f;
                                    POS.y = sealvl + 0.1f;
                                    Vector3D grad;
                                    grad.y = 0.0f;
                                    grad.x = 0.0f;
                                    grad.z = 0.0f;
                                    if (h_map[Y][X] >= sealvl)
                                    {
                                        grad.x -= h_map[Y][X] - sealvl;
                                        grad.z += h_map[Y][X] - sealvl;
                                    }
                                    if (h_map[Y|1][X] >= sealvl)
                                    {
                                        grad.x -= h_map[Y|1][X] - sealvl;
                                        grad.z -= h_map[Y|1][X] - sealvl;
                                    }
                                    if (h_map[Y][X|1] >= sealvl)
                                    {
                                        grad.x += h_map[Y][X|1] - sealvl;
                                        grad.z += h_map[Y][X|1] - sealvl;
                                    }
                                    if (h_map[Y|1][X|1] >= sealvl)
                                    {
                                        grad.x += h_map[Y|1][X|1] - sealvl;
                                        grad.z -= h_map[Y|1][X|1] - sealvl;
                                    }
                                    float grad_len = grad.sq();
                                    if (grad_len > 0.0f)
                                    {
                                        grad = (1.0f / sqrtf( grad_len )) * grad;
                                        fx_manager.addWave(POS, RAD2DEG * ((grad.x >= 0.0f) ? -acosf(grad.z) : acosf(grad.z)));
                                    }
                                }
                            }
                        }
                        bloc[i].point=lvl[pre_y2+x];
                        if (bloc[i].point==NULL)
                        {
                            lvl[pre_y2+x] = bloc[i].point = new Vector3D[9];
                            if(tnt)
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
                            map_data[Y][X].flat = true;
                            for (byte f = 1; f < 9; ++f)			// Check if it's flat
                            {
                                if (bloc[i].point[0].y != bloc[i].point[f].y)
                                {
                                    map_data[Y][X].flat = false;
                                    break;
                                }
                            }
                        }
                    }

                    if (bloc[i].tex != old_tex || buf_size>=500 || ox + 1 < x)
                    {
                        if (buf_size > 0)
                            glDrawRangeElements(GL_TRIANGLE_STRIP, 0, buf_size*9, index_size,GL_UNSIGNED_SHORT,buf_i);		// dessine le tout
                        buf_size = 0;
                        index_size = 0;
                        was_flat = false;
                        if (old_tex != bloc[i].tex)
                        {
                            old_tex=bloc[i].tex;
                            glBindTexture(GL_TEXTURE_2D,bloc[i].tex);
                        }
                    }
                    ox=x;

                    uint16 buf_pos = buf_size * 9;
                    if (!FLAT)
                    {
                        for (byte e = 0; e < 9; ++e) // Copie le bloc
                            buf_p[buf_pos+e]=bloc[i].point[e];
                    }
                    else
                    {
                        for (byte e = 0; e < 9; ++e) // Copie le bloc
                        {
                            buf_p[buf_pos + e].x = flat[e].x + T.x;
                            buf_p[buf_pos + e].y = flat[e].y;
                            buf_p[buf_pos + e].z = flat[e].z + T.z;
                        }
                    }

                    uint8 *color=buf_c+(buf_pos<<2);
                    if( FLAT )
                        for(int e=0;e<36;e+=4)
                        {
                            color[e] = color[e|1] = color[e|2] = 255;
                            color[e|3] = 192;
                        }
                    else
                        for(int e=0;e<36;e+=4)
                            color[e]=color[e|1]=color[e|2]=color[e|3]=255;

                    bool is_clean = true;
                    if( fog_of_war != FOW_DISABLED )
                    {
                        int Z;
                        int grey = 0;
                        int black = 0;
                        Z=Y+get_zdec_notest(X,Y);									    if (Z>=bloc_h_db-1)	Z=bloc_h_db-2;
                        if(!(SurfaceByte(view_map,x,Z>>1) & player_mask))				{	color[0]=color[1]=color[2]=0;	black++;	}
                        else if(!(SurfaceByte(sight_map,x,Z>>1) & player_mask))		    {	color[0]=color[1]=color[2]=127;	grey++;		}
                        if( X + 2 < bloc_w_db )
                        {
                            Z=Y+get_zdec_notest(X+2,Y);								    if (Z>=bloc_h_db-1)	Z=bloc_h_db-2;
                            if(!(SurfaceByte(view_map,x+1,Z>>1) & player_mask))		    {	color[8]=color[9]=color[10]=0;		black++;	}
                            else if(!(SurfaceByte(sight_map,x+1,Z>>1) & player_mask))	{	color[8]=color[9]=color[10]=127;	grey++;		}
                        }
                        if( Y + 2 < bloc_h_db )
                        {
                            Z=Y+2+get_zdec_notest(X,Y+2);							if(Z>=bloc_h_db-1)	Z=bloc_h_db-2;
                            if(!(SurfaceByte(view_map,x,Z>>1) & player_mask))		{	color[24]=color[25]=color[26]=0;	black++;	}
                            else if(!(SurfaceByte(sight_map,x,Z>>1) & player_mask))	{	color[24]=color[25]=color[26]=127;	grey++;		}
                            if( X + 2 < bloc_w_db )
                            {
                                Z=Y+2+get_zdec_notest(X+2,Y+2);							    if(Z>=bloc_h_db-1)	Z=bloc_h_db-2;
                                if(!(SurfaceByte(view_map,x+1,Z>>1) & player_mask))		    {	color[32]=color[33]=color[34]=0;	black++;	}
                                else if(!(SurfaceByte(sight_map,x+1,Z>>1) & player_mask))	{	color[32]=color[33]=color[34]=127;	grey++;		}
                            }
                        }
                        is_clean = grey == 4 || black == 4 || ( grey == 0 && black == 0 );
                        if( !FLAT && !map_data[Y][X].flat && !lp_CONFIG->low_definition_map )
                        {
                            color[4]=color[5]=color[6]= (color[0] + color[8]) >> 1;
                            color[12]=color[13]=color[14]= (color[0] + color[24]) >> 1;
                            color[20]=color[21]=color[22]= (color[8] + color[32]) >> 1;
                            color[16]=color[17]=color[18]= (color[12] + color[20]) >> 1;
                            color[28]=color[29]=color[30]= (color[24] + color[32]) >> 1;
                        }
                    }

                    //#define DEBUG_UNIT_POS

#ifndef DEBUG_UNIT_POS
                    if( FLAT || map_data[Y][X].flat || lp_CONFIG->low_definition_map )
                    {
                        if( was_flat && bloc[i].tex_x == bloc[ bmap[y][x-1] ].tex_x + 1 && is_clean && was_clean && (FLAT || map_data[Y][X].flat) )
                        {
                            buf_i[ index_size-4 ] = 2+buf_pos;
                            buf_i[ index_size-2 ] = 8+buf_pos;
                            buf_i[ index_size-1 ] = 2+buf_pos;
                        }
                        else
                        {
                            buf_i[ index_size++ ] = buf_pos;
                            buf_i[ index_size++ ] = 2+buf_pos;
                            buf_i[ index_size++ ] = 6+buf_pos;
                            buf_i[ index_size++ ] = 8+buf_pos;
                            buf_i[ index_size++ ] = 2+buf_pos;
                            was_flat = FLAT || map_data[Y][X].flat;     // If it's only lp_CONFIG->low_definition_map, it cannot be considered flat
                        }
                    }
                    else
                    {
#endif
                        was_flat = false;
                        buf_i[ index_size++ ] = buf_pos;
                        buf_i[ index_size++ ] = 1+buf_pos;
                        buf_i[ index_size++ ] = 3+buf_pos;
                        buf_i[ index_size++ ] = 4+buf_pos;
                        buf_i[ index_size++ ] = 6+buf_pos;
                        buf_i[ index_size++ ] = 7+buf_pos;
                        buf_i[ index_size++ ] = 7+buf_pos;
                        buf_i[ index_size++ ] = 8+buf_pos;
                        buf_i[ index_size++ ] = 4+buf_pos;
                        buf_i[ index_size++ ] = 5+buf_pos;
                        buf_i[ index_size++ ] = 1+buf_pos;
                        buf_i[ index_size++ ] = 2+buf_pos;
#ifndef DEBUG_UNIT_POS
                    }
#endif
                    was_clean = is_clean;
                    T.x-=x<<4;
                    memcpy(buf_t+(buf_pos<<1),bloc[i].texcoord,72);		// texture

#ifdef DEBUG_UNIT_POS
                    int Z;
                    Z=Y+get_zdec_notest(X,Y);					if(Z>=bloc_h_db-1)	Z=bloc_h_db-2;
                    Z&=0xFFFFFE;
                    X&=0xFFFFFE;
                    if(map_data[Z][X].unit_idx!=-1 )		// Shows unit's pos on map
                    {
                        color[0]=color[1]=color[2]=color[3]=color[4]=color[5]=color[6]=color[7]=color[12]=color[13]=color[14]=color[15]=color[16]=color[17]=color[18]=color[19]=0;
                        if(map_data[Z][X].unit_idx>=0 )		// Shows unit's pos on map
                            color[0]=color[4]=color[12]=color[16]=255;
                        else		// It's a feature
                            color[1]=color[5]=color[13]=color[17]=255;
                    }
                    else if( !map_data[Z][X].air_idx.isEmpty() )		// Shows unit's pos on map
                    {
                        color[0]=color[1]=color[2]=color[3]=color[4]=color[5]=color[6]=color[7]=color[12]=color[13]=color[14]=color[15]=color[16]=color[17]=color[18]=color[19]=0;
                        color[2]=color[6]=color[14]=color[18]=255;
                    }
                    if (map_data[Z][X+1].unit_idx!=-1)		// Shows unit's pos on map
                    {
                        color[8]=color[9]=color[10]=color[11]=color[20]=color[21]=color[22]=color[23]=0;
                        if(map_data[Z][X+1].unit_idx>=0 )		// Shows unit's pos on map
                            color[8]=color[20]=255;
                        else
                            color[9]=color[21]=255;
                    }
                    else if( !map_data[Z][X+1].air_idx.isEmpty() )		// Shows unit's pos on map
                    {
                        color[8]=color[9]=color[10]=color[11]=color[20]=color[21]=color[22]=color[23]=0;
                        color[10]=color[22]=255;
                    }
                    if(map_data[Z+1][X].unit_idx!=-1 )		// Shows unit's pos on map
                    {
                        color[24]=color[25]=color[26]=color[27]=color[28]=color[29]=color[30]=color[31]=0;
                        if(map_data[Z+1][X].unit_idx>=0 )		// Shows unit's pos on map
                            color[24]=color[28]=255;
                        else
                            color[25]=color[29]=255;
                    }
                    else if( !map_data[Z+1][X].air_idx.isEmpty() )		// Shows unit's pos on map
                    {
                        color[24]=color[25]=color[26]=color[27]=color[28]=color[29]=color[30]=color[31]=0;
                        color[26]=color[30]=255;
                    }
                    if(map_data[Z+1][X+1].unit_idx!=-1 )		// Shows unit's pos on map
                    {
                        color[32]=color[33]=color[34]=color[35]=0;
                        if(map_data[Z+1][X+1].unit_idx>=0 )		// Shows unit's pos on map
                            color[32]=255;
                        else
                            color[33]=255;
                    }
                    else if( !map_data[Z+1][X+1].air_idx.isEmpty() )		// Shows unit's pos on map
                    {
                        color[32]=color[33]=color[34]=color[35]=0;
                        color[34]=255;
                    }
#elif defined DEBUG_RADAR_MAP
                    int Z;
                    Z=Y+get_zdec_notest(X,Y);					if(Z>=bloc_h_db-1)	Z=bloc_h_db-2;
                    Z&=0xFFFFFE;
                    X&=0xFFFFFE;
                    if( (radar_map->line[Z>>1][X>>1] & player_mask) )		// Shows unit's pos on map
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
        }
        glDisableClientState(GL_COLOR_ARRAY);		// Couleurs(pour le brouillard de guerre)

        detail_shader.off();
        glActiveTextureARB(GL_TEXTURE1_ARB);
        glClientActiveTextureARB(GL_TEXTURE1_ARB);
        glDisable(GL_TEXTURE_2D);

        glActiveTextureARB(GL_TEXTURE0_ARB);
        glEnable(GL_TEXTURE_2D);
        glClientActiveTextureARB(GL_TEXTURE0_ARB);
        gfx->unlock();
    }




    Vector3D MAP::hit(Vector3D Pos, Vector3D Dir, bool water, float length, bool allow_out)			// Calcule l'intersection d'un rayon avec la carte(le rayon partant du dessus de la carte)
    {
        if (Dir.x == 0.0f && Dir.z == 0.0f) // Solution triviale
        {
            Vector3D P(Pos);
            P.y = get_unit_h(P.x, P.z);
            return P;
        }

        if (get_unit_h(Pos.x,Pos.z) > Pos.y)		// Cas non traité
            return Pos;

        float step = 1.0f;
        if (Dir.x != 0.0f && Dir.z != 0.0f)
        {
            if (fabsf(Dir.x) < fabsf(Dir.z))
                step = 1.0f / fabsf(Dir.x);
            else
                step = 1.0f / fabsf(Dir.z);
        }
        int nb = 0;
        int nb_limit = (int)(Pos.y) + 1000;
        float dwm = map_w_d;
        float dhm = map_h_d;
        Dir = (1.0f * step) * Dir;
        float len_step = Dir.norm();
        while (((sealvl<Pos.y && water) || !water) && get_max_h((int)(Pos.x+map_w_d)>>3,(int)(Pos.z+map_h_d)>>3)<Pos.y)
        {
            if(nb >= nb_limit || length<0.0f)
                return Pos;
            length -= len_step;
            nb++;
            Pos += Dir;
            if ((fabsf(Pos.x) > dwm || fabsf(Pos.z) > dhm) && !allow_out) // Pas de résultat
                return Pos;
        }
        length += len_step;
        Pos -= Dir;

        while(((sealvl<Pos.y && water) || !water) && get_unit_h(Pos.x, Pos.z) < Pos.y)
        {
            if (nb >= nb_limit || length < 0.0f)
                return Pos;
            length -= len_step;
            ++nb;
            Pos += Dir;
            if ((fabsf(Pos.x) > dwm || fabsf(Pos.z)>dhm) && !allow_out) // Pas de résultat
                return Pos;
        }

        for (byte i = 0; i < 7; ++i)
        {
            length += len_step;
            Pos -= Dir; 	// On recommence la dernière opération mais avec plus de précision
            Dir = 0.5f * Dir;
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



    int MAP::check_metal(int x1, int y1, int unit_idx, int *stuff_id )
    {
        if (unit_idx < 0 || unit_idx >= unit_manager.nb_unit)
            return 0;

        int w = unit_manager.unit_type[ unit_idx ]->FootprintX;
        int h = unit_manager.unit_type[ unit_idx ]->FootprintZ;
        int metal_base = 0;
        int end_y = y1 + (h >> 1);
        int end_x = x1 + (w >> 1);
        int start_x = x1 - (w >> 1);
        for (int ry = y1 - (h >> 1 ); ry <= end_y; ++ry)
        {
            if (ry >= 0 && ry < bloc_h_db)
            {
                for( int rx = start_x; rx <= end_x; ++rx)
                {
                    if( rx >= 0 && rx < bloc_w_db )
                    {
                        if( map_data[ry][rx].stuff >=0 )
                        {
                            int type = features.feature[ map_data[ry][rx].stuff ].type;
                            if (!feature_manager.feature[ type ].reclaimable && !feature_manager.feature[ type ].blocking)
                            {
                                metal_base += feature_manager.feature[ type ].metal;
                                if (stuff_id)           // We need to know where to put metal extractors, so it'll give the impression the AI is clever :P
                                    *stuff_id = map_data[ry][rx].stuff;
                            }
                        }
                    }
                }
            }
        }
        if (metal_base == 0)
            metal_base = ota_data.SurfaceMetal;
        return metal_base;
    }



    void WATER::draw(float t, float X, float Y, bool shaded)
    {
        glActiveTextureARB(GL_TEXTURE0_ARB);
        glTranslatef(cosf(t),0.0f,sinf(t));
        if (shaded)
        {
            glBegin(GL_QUADS);
            glTexCoord2f(-map_w/w+0.5f,-map_h/w+0.5f);		glVertex3f(-map_w,0.0f,-map_h);
            glTexCoord2f(map_w/w+0.5f,-map_h/w+0.5f);		glVertex3f(map_w,0.0f,-map_h);
            glTexCoord2f(map_w/w+0.5f,map_h/w+0.5f);		glVertex3f(map_w,0.0f,map_h);
            glTexCoord2f(-map_w/w+0.5f,map_h/w+0.5f);		glVertex3f(-map_w,0.0f,map_h);
            glEnd();
            return;
        }
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f,0.0f);		glVertex3f(-map_w*0.5f,0.0f,-map_h*0.5f);
        glTexCoord2f(1.0f,0.0f);		glVertex3f(map_w*0.5f,0.0f,-map_h*0.5f);
        glTexCoord2f(1.0f,1.0f);		glVertex3f(map_w*0.5f,0.0f,map_h*0.5f);
        glTexCoord2f(0.0f,1.0f);		glVertex3f(-map_w*0.5f,0.0f,map_h*0.5f);

        glDisable( GL_TEXTURE_2D );
        glColor4f(0.0f,0.0f,0.0f,1.0f);

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



    void SKY::build(int d,float size, bool full_sphere)
    {
        destroy();

        full = full_sphere;

        s = full_sphere ? (d << 1) : d;
        w = size;

        nb_vtx = (s + 1) * ((s << 1) + 1);
        nb_idx = s * (s * 2 + 1) * 2; // We'll use GL_TRIANGLE_STRIP

        point = new Vector3D[nb_vtx];
        texcoord = new float[nb_vtx*2];
        index = new GLushort[nb_idx];

        int i=0;
        for (int y = 0; y <= s; ++y)
        {
            for (int x = 0; x <= s * 2; ++x)
            {
                if (full_sphere)
                {
                    point[i].x = size * cosf(x / (2.0f * s) * PI * 2.0f) * cosf((float)y / s * PI - PI * 0.5f);
                    point[i].y = size * sinf((float)y / s * PI - PI * 0.5f);
                    point[i].z = -size * sinf(x / (2.0f * s) * PI * 2.0f) * cosf((float)y / s * PI - PI * 0.5f);
                }
                else
                {
                    point[i].x = size * cosf(x / (2.0f * s) * PI * 2.0f) * cosf(0.5f * y / s * PI);
                    point[i].y = size * sinf(0.5f * y / s * PI);
                    point[i].z = -size * sinf(x / (2.0f * s) * PI * 2.0f) * cosf(0.5f * y / s * PI);
                }
                texcoord[i << 1] = (float)x / (s * 2);
                texcoord[(i << 1) + 1] = 1.0f - (float)y / s;
                ++i;
            }
        }
        i = 0;
        for (int y = 0; y < s; ++y)	// We'll use GL_TRIANGLE_STRIP
        {
            for (int x = 0; x <= s * 2; ++x)
            {
                index[i++] = y * (s * 2 + 1) + x;
                index[i++] = (y + 1)*(s * 2 + 1) + x;
            }
        }
    }


    void SKY::draw()
    {
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
        glVertexPointer(3, GL_FLOAT, 0, point);
        glClientActiveTextureARB(GL_TEXTURE0_ARB);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, 0, texcoord);
        glDrawRangeElements(GL_TRIANGLE_STRIP, 0, nb_vtx-1, nb_idx,GL_UNSIGNED_SHORT,index);		// dessine le tout
    }


    void SKY_DATA::load_tdf(const String& filename)
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
        parser.pullAsString("sky.planet").split(planet, ",");
        FogColor[0] = parser.pullAsFloat("sky.fog R");
        FogColor[1] = parser.pullAsFloat("sky.fog G");
        FogColor[2] = parser.pullAsFloat("sky.fog B");
        FogColor[3] = parser.pullAsFloat("sky.fog A");
        parser.pullAsString("sky.map").split(MapName, ",");
    }


    SKY_DATA* choose_a_sky(const String& mapname, const String& planet)
    {
        std::list<SKY_DATA*> sky_list;
        sky_list.clear();

        String::List file_list;
        HPIManager->getFilelist("sky\\*.tdf", file_list);
        uint32	nb_sky = 0;

        for (String::List::const_iterator it = file_list.begin(); it != file_list.end(); ++it)
        {
            LOG_DEBUG("loading sky : " << *it);
            SKY_DATA *sky_data = new SKY_DATA;
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
                for (String::Vector::const_iterator i = sky_data->planet.begin(); i != sky_data->planet.end(); ++i)
                {
                    if (*i == planet)
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

        if( nb_sky == 0 )// Look for a default sky
        {
            for (String::List::const_iterator it = file_list.begin(); it != file_list.end(); ++it)
            {
                SKY_DATA *sky_data = new SKY_DATA;
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

        SKY_DATA *selected_sky = NULL;

        if (nb_sky > 0)
        {
            int select = TA3D_RAND() % nb_sky;
            for (std::list<SKY_DATA*>::iterator it = sky_list.begin() ; it != sky_list.end(); ++it, --select)
                if( select == 0 )
                {
                    selected_sky = *it;
                    *it = NULL;
                    break;
                }
        }

        for (std::list<SKY_DATA*>::iterator it = sky_list.begin() ; it != sky_list.end(); ++it)
        {
            if( *it != NULL )
                delete *it;
        }
        sky_list.clear();

        return selected_sky;
    }



    SKY_DATA::SKY_DATA()
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

    SKY_DATA::~SKY_DATA()
    {
        texture_name.clear();
        planet.clear();
        MapName.clear();
    }


} // namespace TA3D

