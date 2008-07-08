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

/*----------------------------------------------------------\
|                      EngineClass.h                        |
|    Contient toutes les classes nécessaires au moteur 3D   |
| et au moteur physique.                                    |
|                                                           |
\----------------------------------------------------------*/

#ifndef __TA3D_ENGINE_CL_H__
# define __TA3D_ENGINE_CL_H__


# include "tdf.h"				// Pour le gestionnaire de sprites
# include "threads/thread.h"
# include "gfx/particles/particles.h"
# include "ai/ai.h"
# include "network/TA3D_Network.h"
# include "misc/camera.h"
# include <vector>

#define PARTICLE_LIMIT		100000		// pas plus de 100000 particules
#define HMAP_RESIZE			0.04f

#define H_DIV		0.5f


namespace TA3D
{


    float const tnt_transform=1.0f/tan(63.44f*DEG2RAD)/H_DIV;
    float const tnt_transform_H_DIV=1.0f/tan(63.44f*DEG2RAD);





    class IDX_LIST_NODE			// Node of the list
    {
    public:
        sint16			idx;
        IDX_LIST_NODE	*next;

        IDX_LIST_NODE( sint16 n_idx )	{	idx = n_idx;	next = NULL;	}
        IDX_LIST_NODE( sint16 n_idx, IDX_LIST_NODE *n_next )	{	idx = n_idx;	next = n_next;	}
        IDX_LIST_NODE()	{	idx = 0;	next = NULL;	}
    };

    class IDX_LIST				// Container
    {
    public:
        IDX_LIST_NODE	*head;

        void init()	{	head = NULL;	}
        IDX_LIST()	{	init();	}

        void destroy()
        {
            while( head ) {
                IDX_LIST_NODE *next = head->next;
                delete head;
                head = next;
            }
        }

        ~IDX_LIST()	{	destroy();	}

        inline bool isEmpty()	{	return head == NULL;	}

        inline void push( sint16 idx )
        {
            if( head ) {
                IDX_LIST_NODE *cur = head;
                while( cur->next ) {
                    if( cur->idx == idx )	return;		// Don't add it twice
                    cur = cur->next;
                }
                cur->next = new IDX_LIST_NODE( idx );	// Add idx at the end
            }
            else
                head = new IDX_LIST_NODE( idx );
        }

        inline void remove( sint16 idx )		// Assume there is only one occurence of idx in the list
        {
            IDX_LIST_NODE *cur = head;
            IDX_LIST_NODE *prec = NULL;
            while( cur ) {
                if( cur->idx == idx ) {
                    if( prec == NULL ) {
                        prec = head;	head = head->next;
                        delete prec;	return;
                    }
                    else {
                        prec->next = cur->next;
                        delete cur;	return;
                    }
                }
                prec = cur;
                cur = cur->next;
            }
        }

        inline bool isIn( sint16 idx )
        {
            IDX_LIST_NODE *cur = head;
            while( cur ) {
                if( cur->idx == idx )	return true;
                cur = cur->next;
            }
            return false;
        }
    };





    class SECTOR			// Structure pour regrouper les informations sur le terrain (variations d'altitude, submergé, teneur en metal, ...)
    {
    public:
        float		dh;					// dérivée maximale en valeur absolue de l'altitude
        bool		underwater;			// indique si le bloc est sous l'eau
        sint32		stuff;				// Indique l'élément graphique présent sur ce secteur
        sint16		unit_idx;			// Indice de l'unité qui se trouve sur ce secteur
        bool		lava;				// Is that under lava ?? Used for pathfinding
        IDX_LIST	air_idx;			// This is the list that stores indexes of air units
        bool		flat;				// Used by the map renderer to simplify geometry

        void init()
        {
            dh = 0.0f;
            underwater = false;
            stuff = -1;
            unit_idx = -1;
            lava = false;
            air_idx.init();
            flat = false;
        }
    };


}

#include "ai/pathfinding.h"		// Algorithme de pathfinding

// FIXME !

namespace TA3D
{

    class MAP_OTA
    {
    public:
        char	*missionname;
            char	*planet;
            char	*missiondescription;
        char	*glamour;
            int		tidalstrength;
            int		solarstrength;
            bool	lavaworld;
            short	killmul;
            int		minwindspeed;
            int		maxwindspeed;
            float	gravity;
            char	*numplayers;
            char	*map_size;
            int		SurfaceMetal;
            int		MohoMetal;
            int		startX[10];
            int		startZ[10];
            bool	waterdoesdamage;
            int		waterdamage;
        bool	network;
        
            void init()
            {
                network = false;
                    planet=NULL;
                glamour=NULL;
                    missionname=NULL;
                    missiondescription=NULL;
                    numplayers=NULL;
                    map_size=NULL;
                    for(int i=0;i<10;i++)
                        startX[i]=startZ[i]=0;
                            tidalstrength=0;
                            solarstrength=22;
                            lavaworld=false;
                            killmul=50;
                            minwindspeed=0;
                            maxwindspeed=0;
                            gravity=9.8f;
                            SurfaceMetal=0;
                            MohoMetal=0;
                            waterdamage=0;
                waterdoesdamage=false;
            }

        void destroy()
        {
            if(glamour)				free(glamour);
            if(planet)				free(planet);
            if(missionname)			free(missionname);
            if(missiondescription)	free(missiondescription);
            if(numplayers)			free(numplayers);
            if(map_size)			free(map_size);
            init();
        }

        MAP_OTA()
        {
            init();
        }

        ~MAP_OTA()
        {
            destroy();
        }

    private:
        inline char *get_line(char *data)
        {
            int pos=0;
            while(data[pos]!=0 && data[pos]!=13 && data[pos]!=10)	pos++;
            char *d=new char[pos+1];
            memcpy(d,data,pos);
            d[pos]=0;
            return d;
        }
    public:

        void load(char *data,int ota_size);
        void load( String filename );
    };

    class BLOC				// Blocs composant la carte
    {
    public:
        byte		nbindex;	// Nombre d'indices	/ Number of indexes
        byte		nbpoint;	// Nombre de points / Number of points
        VECTOR		*point;		// Points du bloc / Array of points
        float		*texcoord;	// Coordonnées de texture / Texture coordinates
        GLuint		tex;		// Indice de texture OpenGl / OpenGL texture handle
        bool		lava;		// Indique si le bloc est de type lave / Is that a lava bloc ?
        byte		tex_x;

        void init()
        {
            nbindex=nbpoint=0;
            point=NULL;
            texcoord=NULL;
            tex=0;
            lava=false;
            tex_x=0;
        }

        BLOC()
        {
            init();
        }

        void destroy()
        {
            if(point) delete[] point;
            if(texcoord) delete[] texcoord;
            init();
        }
    };

    /*------------------- Here is the two classes of a list lighter than a List< sint16 > ----------*/
    class MAP : public ObjectSync // Données concernant la carte
    {
    public:
        short		ntex;			// Indique si la texture est chargée et doit être détruite
        GLuint		*tex;			// Texture de surface
        int			nbbloc;			// Nombre de blocs
        BLOC		*bloc;			// Blocs composant le terrain
        unsigned short	**bmap;		// Tableau d'indice des blocs
        float		**h_map;		// Tableau de l'élévation du terrain
        float		**ph_map;		// Tableau du relief projeté pour le calcul inverse(projection) lors de l'affichage
        byte		**ph_map_2;		// Tableau du relief projeté (multiplié par un facteur flottant) pour le calcul inverse(projection) lors de l'affichage
        SECTOR		**map_data;		// Tableau d'informations sur le terrain
        byte		**view;			// Indique quels sont les parcelles de terrain visibles à l'écran
        byte		**path;			// Tableau pour le pathfinding

        BITMAP		*view_map;		// Map of what has been discovered
        BITMAP		*sight_map;		// Map of who is viewing
        BITMAP		*radar_map;		// Radar map
        BITMAP		*sonar_map;		// Sonar map

        int			map_w;			// Largeur de la carte en 16ème de bloc
        int			map_h;			// Hauteur de la carte en 16ème de bloc
        int			map_w_d;		// Same values as above divided by 2
        int			map_h_d;
        int			bloc_w;			// Width in blocs
        int			bloc_h;			// Height in blocs
        int			bloc_w_db;		// Same as above but multiplied by 2
        int			bloc_h_db;
        float		map2blocdb_w;
        float		map2blocdb_h;
        BITMAP		*mini;			// Minimap
        GLuint		glmini;			// Texture OpenGl pour la minimap
        int			mini_w;
        int			mini_h;
        float		sealvl;			// Niveau de la mer
        VECTOR		**lvl;			// Bloc de flottants pour le relief de la carte
        bool		water;			// Indique qu'il faut dessiner la mer
        bool		tnt;			// Indique si la carte est format tnt(format de total annihilation)
        float		sea_dec;		// Décalage de la mer
        int			ox1,ox2;		// Coordonnées de la dernière fenêtre de terrain dessinée
        int			oy1,oy2;
        GLushort	buf_i[6500];	// Pour accélérer l'affichage
        GLuint		lava_map;		// texture des zones de lave
        GLuint		details_tex;	// details texture to show more details when zooming on the map
        float		color_factor;	// color factor used when details_tex is set with a texture that darken the map
        SHADER		detail_shader;	// pixel shader to add the detail texture correctly

        MAP_OTA		ota_data;		// Data read from the ota file

        float		wind;			// To handle wind
        float		wind_dir;
        VECTOR		wind_vec;

        /*------------- Experimental: code for new map format -----------------------*/

        /*	BLOC		**macro_bloc;	// map
            uint32		macro_w;
            uint32		macro_h;*/

        /*---------------------------------------------------------------------------*/

        int			low_nb_idx;
        int			low_w,low_h;
        VECTOR		*low_vtx;
        VECTOR		*low_vtx_flat;
        float		*low_tcoord;
        uint8		*low_col;
        GLuint		*low_index;
        GLuint		low_tex;

        uint8		fog_of_war;

        void clear_FOW( sint8 FOW_flags = -1 );

        void load_details_texture( const String &filename );

        void init()
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
            for(short i=0;i<6500;i++) {
                buf_i[i++]=0+buf_size;
                buf_i[i++]=1+buf_size;
                buf_i[i++]=3+buf_size;
                buf_i[i++]=4+buf_size;
                buf_i[i++]=6+buf_size;
                buf_i[i++]=7+buf_size;
                buf_i[i++]=7+buf_size;
                buf_i[i++]=8+buf_size;
                buf_i[i++]=4+buf_size;
                buf_i[i++]=5+buf_size;
                buf_i[i++]=1+buf_size;
                buf_i[i++]=2+buf_size;
                buf_i[i]=2+buf_size;
                buf_size+=9;
            }
        }

        MAP()
        {
            init();
        }

        void destroy();

        void clean_map();		// Used to remove all objects when loading a saved game

        ~MAP()
        {
            destroy();
        }

        void update_player_visibility( int player_id, int px, int py, int r, int rd, int sn, int rd_j, int sn_j, bool jamming=false, bool black=false );	// r -> sight, rd -> radar range, sn -> sonar range, j for jamming ray

        void draw_mini(int x1=0,int y1=0,int w=252,int h=252, Camera *cam=NULL, byte player_mask=0xFF ); // Dessine la mini-carte

        inline const float get_unit_h(float x,float y)
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
            if(Y+1<ly) {
                h[2]=h_map[Y+1][X];
                if(X+1<lx)
                    h[3]=h_map[Y+1][X+1]-h[2];
                else
                    h[3]=0.0f;
            }
            else {
                h[2]=h[0];
                h[3]=h[1];
            }
            h[0]=h[0]+h[1]*dx;
            return h[0]+(h[2]+h[3]*dx-h[0])*dy;
        }

        inline float get_h(int x,int y)
        {
            if(x<0) x=0;
            if(y<0) y=0;
            if(x>=bloc_w_db-1) x=bloc_w_db-2;
            if(y>=bloc_h_db-1) y=bloc_h_db-2;
            return h_map[y][x];
        }

        inline float get_max_h(int x,int y)
        {
            if(x<0) x=0;
            if(y<0) y=0;
            if(x>=bloc_w_db-1) x=bloc_w_db-2;
            if(y>=bloc_h_db-1) y=bloc_h_db-2;
            float h = h_map[y][x];
            if(x<bloc_w_db-2)	h = max(h, h_map[y][x+1]);
            if(y<bloc_h_db-2) {
                h = max(h, h_map[y+1][x]);
                if(x<bloc_w_db-2)	h = max(h, h_map[y+1][x+1]);
            }
            return h;
        }

        inline float get_max_rect_h(int x,int y, int w, int h)
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
            for( int Y = y1 ; Y <= y2 ; Y++ )
                for( int X = x1 ; X <= x2 ; X++ ) {
                    float h = h_map[Y][X];
                    if( h > max_h )	max_h = h;
                }
            return max_h;
        }

        inline float get_zdec(int x,int y)
        {
            if(x<0) x=0;
            if(y<0) y=0;
            if(x>=bloc_w_db-1) x=bloc_w_db-2;
            if(y>=bloc_h_db-1) y=bloc_h_db-2;
            return ph_map[y][x]*tnt_transform_H_DIV;
        }

#define get_zdec_notest(x, y)	ph_map_2[y][x]

        /*	inline byte get_zdec_notest(int x,int y)
            {
            return ph_map_2[y][x];
            }*/

        inline float get_nh(int x,int y)
        {
            if(x<0) x=0;
            if(y<0) y=0;
            if(x>=bloc_w_db-1) x=bloc_w_db-2;
            if(y>=bloc_h_db-1) y=bloc_h_db-2;
            return ph_map[y][x];
        }

        inline void rect(int x1,int y1,int w,int h,short c,char *yardmap=NULL,bool open=false)
        {
            if(yardmap==NULL) {
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
            else {
                int i=0;
                int y2=y1+h;
                int x2=x1+w;
                if(y1<0)	{	i-=y1*w;	y1=0;	}
                if(y2>bloc_h_db-1)	y2=bloc_h_db-1;
                if(x1<0)	x1=0;
                if(x2>bloc_w_db-1)	x2=bloc_w_db-1;
                int dw=w-(x2-x1);
                if(y2<=y1 || x2<=x1)	return;
                pMutex.lock();
                for(int y=y1;y<y2;y++) {
                    for(int x=x1;x<x2;x++) {
                        if( !yardmap[i] ) {
                            pMutex.unlock();
                            return;
                        }
                        if(yardmap[i]=='G' || yardmap[i]=='o' || yardmap[i]=='w' || yardmap[i]=='f' || (yardmap[i]=='c' && !open) || (yardmap[i]=='C' && !open) || (yardmap[i]=='O' && open))
                            map_data[y][x].unit_idx=c;
                        i++;
                    }
                    i+=dw;
                }
                pMutex.unlock();
            }
        }

        inline void air_rect( int x1, int y1, int w, int h, short c, bool remove = false )
        {
            if( remove ) {
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
            else {
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
                        map_data[y][x].air_idx.push(c);
                pMutex.unlock();
            }
        }

        inline bool check_rect(int x1,int y1,int w,int h,short c)
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

        inline bool check_rect_discovered(int x1,int y1,int w,int h,short c)		// Check if the area has been fully discovered
        {
            int y2=y1+h+1>>1;
            int x2=x1+w+1>>1;
            x1>>=1;
            y1>>=1;
            if(y1<0)	y1=0;
            if(y2>bloc_h-1)	y2=bloc_h-1;
            if(x1<0)	x1=0;
            if(x2>bloc_w-1)	x2=bloc_w-1;
            if(y2<=y1 || x2<=x1)	return false;
            for(int y=y1;y<y2;y++)
                for(int x=x1;x<x2;x++)
                    if( !(view_map->line[y][x] & c) )
                        return false;
            return true;
        }

        inline float check_rect_dh(int x1,int y1,int w,int h)
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
            for(int y=y1;y<y2;y++)
                for(int x=x1;x<x2;x++) {
                    if(map_data[y][x].dh>max_dh)
                        max_dh=map_data[y][x].dh;
                    on_water|=map_data[y][x].underwater;
                }
            if(on_water)
                max_dh=-max_dh;
            return max_dh;
        }

        inline float check_max_depth(int x1,int y1,int w,int h)
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
                for(int x=x1;x<x2;x++) {
                    float d = -h_map[y][x];
                    if(d>depth)
                        depth=d;
                }
            return depth + sealvl;
        }

        inline float check_min_depth(int x1,int y1,int w,int h)
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
                for(int x=x1;x<x2;x++) {
                    float d = -h_map[y][x];
                    if(d<depth)
                        depth=d;
                }
            return depth+sealvl;
        }

        inline bool check_vents(int x1,int y1,int w,int h,char *yard_map)
        {
            if(yard_map==NULL)	return true;
            int y2=y1+h;
            int x2=x1+w;
            if(y1<0)	y1=0;
            if(y2>bloc_h_db-1)	y2=bloc_h_db-1;
            if(x1<0)	x1=0;
            if(x2>bloc_w_db-1)	x2=bloc_w_db-1;
            int dw = w - (x2-x1);
            int i = 0;
            bool ok = true;
            if(y2<=y1 || x2<=x1)	return false;
            for(int y=y1;y<y2;y++) {
                for(int x=x1;x<x2;x++) {
                    if( !yard_map[i] )	return ok;
                    if(yard_map[i]=='G') {
                        ok = false;
                        if(map_data[y][x].stuff>=0) {
                            int feature_id = map_data[y][x].stuff;
                            if(feature_manager.feature[features.feature[feature_id].type].geothermal)
                                return true;
                        }
                    }
                    i++;
                }
                i+=dw;
            }
            return ok;
        }

        inline bool check_lava(int x1,int y1,int w,int h)
        {
            int y2=y1+h;
            int x2=x1+w;
            if(y1<0)	y1=0;
            if(y2>bloc_h-1)	y2=bloc_h-1;
            if(x1<0)	x1=0;
            if(x2>bloc_w-1)	x2=bloc_w-1;
            if(y2<=y1 || x2<=x1)	return false;
            for(int y=y1;y<y2;y++)
                for(int x=x1;x<x2;x++)
                    if(bloc[bmap[y][x]].lava)
                        return true;
            return false;
        }

        int check_metal(int x1, int y1, int unit_idx );

        void draw(Camera* cam,byte player_mask,bool FLAT=false,float niv=0.0f,float t=0.0f,float dt=1.0f,bool depth_only=false,bool check_visibility=true,bool draw_uw=true);

        VECTOR hit(VECTOR Pos,VECTOR Dir,bool water = true, float length = 200000.0f, bool allow_out = false);			// Calcule l'intersection d'un rayon avec la carte(le rayon partant du dessus de la carte)
    };

    extern MAP *the_map;

    class WATER
    {
    public:
        float		map_w;
        float		map_h;
        float		w;

        void init()
        {
            map_w = 0.0f;
            map_h = 0.0f;
            w = 1.0f;
        }

        void destroy()
        {
            init();
        }

        WATER()
        {
            init();
        }

        ~WATER()
        {
            destroy();
        }

        void build( float m_w, float m_h, float size )
        {
            destroy();
            map_w = m_w;
            map_h = m_h;
            w = size;
        }

        void draw(float t,float X,float Y,bool shaded=false);
    };

    class SKY
    {
    public:
        uint16		nb_vtx;
        uint16		nb_idx;
        VECTOR		*point;
        float		*texcoord;
        GLushort	*index;
        int			s;
        float		w;
        bool		full;

        void init()
        {
            full = false;
            nb_vtx=0;
            nb_idx=0;
            point=NULL;
            texcoord=NULL;
            index=NULL;
            s=0;
            w=1.0f;
        }

        void destroy()
        {
            if(point)		free(point);
            if(texcoord)	free(texcoord);
            if(index)		free(index);
            init();
        }

        SKY()
        {
            init();
        }

        ~SKY()
        {
            destroy();
        }

        void build(int d,float size, bool full_sphere=false);

        void draw();
    };

    extern int NB_PLAYERS;

#define PLAYER_CONTROL_LOCAL_HUMAN	0x0
#define PLAYER_CONTROL_REMOTE_HUMAN	0x1
#define PLAYER_CONTROL_LOCAL_AI		0x2
#define PLAYER_CONTROL_REMOTE_AI	0x3
#define PLAYER_CONTROL_NONE			0x4
#define PLAYER_CONTROL_CLOSED		0x8

#define PLAYER_CONTROL_FLAG_REMOTE	0x1
#define PLAYER_CONTROL_FLAG_AI		0x2

    class PLAYERS :			public ObjectSync,			// The player control/management class
    public cThread						// Classe pour gérer les joueurs et leurs statistiques de partie
    {
    public:
        sint8		nb_player;		// Nombre de joueurs (maximum 10 joueurs)
        int			local_human_id;	// Quel est le joueur qui commande depuis cette machine??
        byte		control[10];	// Qui controle ce joueur??
        char		*nom[10];		// Noms des joueurs
        char		*side[10];		// Camp des joueurs
        float		energy[10];		// Energie des joueurs
        float		metal[10];		// Metal des joueurs
        float		metal_u[10];	// Metal utilisé
        float		energy_u[10];	// Energie utilisée
        float		metal_t[10];	// Metal extrait
        float		energy_t[10];	// Energie produite
        uint32		kills[10];		// Victimes
        uint32		losses[10];		// Pertes
        uint32		energy_s[10];	// Capacités de stockage d'énergie
        uint32		metal_s[10];	// Capacités de stockage de metal
        uint32		com_metal[10];	// Stockage fournit par le commandeur
        uint32		com_energy[10];
        bool		commander[10];	// Indique s'il y a un commandeur
        bool		annihilated[10];// Le joueur a perdu la partie??
        AI_PLAYER	*ai_command;	// Controleurs d'intelligence artificielle
        uint32		nb_unit[10];	// Nombre d'unités de chaque joueur
        uint8		side_view;		// Side of which we draw the game interface

        //		Variables used to compute the data we need ( because of threading )

        float		c_energy[10];		// Energie des joueurs
        float		c_metal[10];		// Metal des joueurs
        uint32		c_energy_s[10];		// Capacités de stockage d'énergie
        uint32		c_metal_s[10];		// Capacités de stockage de metal
        bool		c_commander[10];	// Indique s'il y a un commandeur
        bool		c_annihilated[10];	// Le joueur a perdu la partie??
        uint32		c_nb_unit[10];		// Nombre d'unités de chaque joueur
        float		c_metal_u[10];		// Metal utilisé
        float		c_energy_u[10];		// Energie utilisée
        float		c_metal_t[10];		// Metal extrait
        float		c_energy_t[10];		// Energie produite

        // For statistic purpose only
        double		energy_total[10];
        double		metal_total[10];

    protected:
        uint32		last_ticksynced;
        TA3DNetwork	*ta3d_network;
        MAP			*map;
        bool		thread_is_running;
        bool		thread_ask_to_stop;
        int			Run();
        void		SignalExitThread();
    public:

        inline void set_network( TA3DNetwork *net )	{	ta3d_network = net;	}

        inline void set_map( MAP *p_map )	{	map = p_map;	}

        void player_control();

        inline void stop_threads()
        {
            for( byte i = 0 ; i < nb_player ; i++ )
                if( control[ i ] == PLAYER_CONTROL_LOCAL_AI && ai_command )
                    ai_command[ i ].DestroyThread();
        }

        inline void clear()		// Remet à 0 la taille des stocks
        {
            for(byte i=0;i<nb_player;i++) {
                c_energy[i] = energy[i];
                c_metal[i] = metal[i];
                c_energy_s[i]=c_metal_s[i]=0;			// Stocks
                c_metal_t[i]=c_energy_t[i]=0.0f;		// Production
                c_metal_u[i]=c_energy_u[i]=0.0f;		// Consommation
                c_commander[i]=false;
                c_annihilated[i]=true;
                c_nb_unit[i]=0;
            }
        }

        inline void refresh()		// Copy the newly computed values over old ones
        {
            for(byte i=0;i<nb_player;i++) {
                energy[i] = c_energy[i];
                metal[i] = c_metal[i];
                energy_s[i] = c_energy_s[i];
                metal_s[i] = c_metal_s[i];				// Stocks
                commander[i] = c_commander[i];
                annihilated[i] = c_annihilated[i];
                nb_unit[i] = c_nb_unit[i];
                metal_t[i] = c_metal_t[i];
                energy_t[i] = c_energy_t[i];
                metal_u[i] = c_metal_u[i];
                energy_u[i] = c_energy_u[i];
            }
        }

        int add(char *NOM,char *SIDE,byte _control,int E=10000,int M=10000,byte AI_level=AI_TYPE_EASY);

        void show_resources();

        inline void init(int E=10000,int M=10000)		// Initialise les données des joueurs
        {
            ta3d_network = NULL;
            side_view = 0;
            nb_player=0;
            NB_PLAYERS=0;
            local_human_id=-1;
            clear();
            refresh();
            map = NULL;
            for(int i=0;i<10;i++) {
                com_metal[i] = M;
                com_energy[i] = E;
                control[i] = PLAYER_CONTROL_NONE;
                if( ai_command ) {
                    ai_command[i].init();
                    ai_command[i].player_id = i;
                }
                nom[i] = NULL;
                side[i] = NULL;
                energy[i] = E;
                metal[i] = M;
                metal_u[i] = 0;
                energy_u[i] = 0;
                metal_t[i] = 0;
                energy_t[i] = 0;
                kills[i] = 0;
                losses[i] = 0;
                energy_s[i] = E;
                metal_s[i] = M;
                nb_unit[i] = 0;
                energy_total[i] = 0.0f;
                metal_total[i] = 0.0f;

                c_energy[i] = energy[i];
                c_metal[i] = metal[i];
                c_energy_s[i]=c_metal_s[i]=0;			// Stocks
                c_metal_t[i]=c_energy_t[i]=0.0f;		// Production
                c_metal_u[i]=c_energy_u[i]=0.0f;		// Consommation
                c_commander[i]=false;
                c_annihilated[i]=true;
                c_nb_unit[i]=0;
            }
        }

        inline PLAYERS()
        {
            map = NULL;
            thread_is_running = false;
            thread_ask_to_stop = false;

            InitThread();

            ai_command = new AI_PLAYER[ 10 ];
            init();
        }

        inline void destroy()
        {
            for(byte i=0;i<10;i++) {
                if(control[i]==PLAYER_CONTROL_LOCAL_AI && ai_command )		// Enregistre les données de l'IA
                    ai_command[i].save();
                if(nom[i])	free(nom[i]);
                if(side[i])	free(side[i]);
                if( ai_command )
                    ai_command[i].destroy();
            }
            init();
        }

        inline ~PLAYERS()
        {
            destroy();
            if( ai_command )
                delete[] ai_command;
            ai_command = NULL;

            DestroyThread();
        }
    };

    extern PLAYERS	players;		// Objet contenant les données sur les joueurs

    class SKY_DATA
    {
    public:
        bool			spherical;			// Flat or spherical sky
        float			rotation_speed;		// For spherical sky
        float			rotation_offset;	// If you want the sun to match light dir ...
        String			texture_name;		// Name of the texture used as sky
        std::vector<String>	planet;				// Vector of planets that can use this sky
        float			FogColor[4];		// Color of the fog to use with this sky
        std::vector<String>	MapName;			// Name of maps linked with this sky
        bool			full_sphere;		// The texture is for the whole sphere
        bool			def;

        SKY_DATA()
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

        ~SKY_DATA()
        {
            texture_name.clear();
            planet.clear();
            MapName.clear();
        }

        void load_tdf( const String	&filename );
    };

    SKY_DATA	*choose_a_sky( const String &mapname, const String &planet );


}

#endif // __TA3D_ENGINE_CL_H__
