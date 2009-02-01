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
# include "misc/math.h"


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

    /*!
    ** \todo This class should be removed
    */
    class IDX_LIST				// Container
    {
    public:
        IDX_LIST_NODE	*head;

        void init()	{ head = NULL; }
        IDX_LIST()	{ init(); }

        void destroy();

        ~IDX_LIST()	{destroy();}

        bool isEmpty() const {return (!head);}

        void push(const sint16 idx);

        void remove(const sint16 idx);		// Assume there is only one occurence of idx in the list

        bool isIn(const sint16 idx);

    }; // class IDX_LIST





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

        void init();
    };


}

#include "ai/pathfinding.h"		// Algorithme de pathfinding

// FIXME !

namespace TA3D
{



    class MAP_OTA
    {
    public:
        MAP_OTA() {init();}
        ~MAP_OTA() {destroy();}

        void init();

        void destroy();

        void load(char *data,int ota_size);
        void load( String filename );

    public:
        String  missionname;
        String  planet;
        String  missiondescription;
        String  glamour;
        int     tidalstrength;
        int     solarstrength;
        bool    lavaworld;
        short   killmul;
        int     minwindspeed;
        int     maxwindspeed;
        float   gravity;
        String  numplayers;
        String  map_size;
        int     SurfaceMetal;
        int	    MohoMetal;
        int     startX[TA3D_PLAYERS_HARD_LIMIT];
        int     startZ[TA3D_PLAYERS_HARD_LIMIT];
        bool    waterdoesdamage;
        int	    waterdamage;
        bool    network;

    private:
        //! \todo Must be removed
        char *get_line(char *data);

    };



    class BLOC				// Blocs composant la carte
    {
    public:
        byte		nbindex;	// Nombre d'indices	/ Number of indexes
        byte		nbpoint;	// Nombre de points / Number of points
        Vector3D	*point;		// Points du bloc / Array of points
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
        Vector3D	**lvl;			// Bloc de flottants pour le relief de la carte
        bool		water;			// Indique qu'il faut dessiner la mer
        bool		tnt;			// Indique si la carte est format tnt(format de total annihilation)
        float		sea_dec;		// Décalage de la mer
        int			ox1,ox2;		// Coordonnées de la dernière fenêtre de terrain dessinée
        int			oy1,oy2;
        GLushort	buf_i[6500];	// Pour accélérer l'affichage
        GLuint		lava_map;		// texture des zones de lave
        GLuint		details_tex;	// details texture to show more details when zooming on the map
        float		color_factor;	// color factor used when details_tex is set with a texture that darken the map
        Shader		detail_shader;	// pixel shader to add the detail texture correctly

        MAP_OTA		ota_data;		// Data read from the ota file

        float		wind;			// To handle wind
        float		wind_dir;
        Vector3D	wind_vec;

        /*------------- Experimental: code for new map format -----------------------*/

        /*	BLOC		**macro_bloc;	// map
            uint32		macro_w;
            uint32		macro_h;*/

        /*---------------------------------------------------------------------------*/

        int			low_nb_idx;
        int			low_w,low_h;
        Vector3D	*low_vtx;
        Vector3D	*low_vtx_flat;
        float		*low_tcoord;
        uint8		*low_col;
        GLuint		*low_index;
        GLuint		low_tex;

        uint8		fog_of_war;

        void clear_FOW( sint8 FOW_flags = -1 );

        void load_details_texture( const String &filename );

        void init();

        MAP() {init();}

        void destroy();

        void clean_map();		// Used to remove all objects when loading a saved game

        ~MAP() {destroy();}

        void check_unit_visibility(int x, int y);

        void update_player_visibility( int player_id, int px, int py, int r, int rd, int sn, int rd_j, int sn_j, bool jamming=false, bool black=false );	// r -> sight, rd -> radar range, sn -> sonar range, j for jamming ray

        void draw_mini(int x1=0,int y1=0,int w=252,int h=252, Camera *cam=NULL, byte player_mask=0xFF ); // Dessine la mini-carte

        float get_unit_h(float x,float y);

        float get_h(int x,int y);

        float get_max_h(int x,int y);

        float get_max_rect_h(int x,int y, int w, int h);

        float get_zdec(int x,int y);

#define get_zdec_notest(x, y)	ph_map_2[y][x]

        float get_nh(int x,int y);

        void rect(int x1,int y1,int w,int h,short c,const String &yardmap = String(),bool open=false);

        void air_rect( int x1, int y1, int w, int h, short c, bool remove = false);

        bool check_rect(int x1,int y1,int w,int h,short c);

        bool check_rect_discovered(int x1,int y1,int w,int h,short c); // Check if the area has been fully discovered

        float check_rect_dh(int x1,int y1,int w,int h);

        float check_max_depth(int x1,int y1,int w,int h);

        float check_min_depth(int x1,int y1,int w,int h);

        bool check_vents(int x1,int y1,int w,int h,const String &yard_map);

        bool check_lava(int x1,int y1,int w,int h);

        int check_metal(int x1, int y1, int unit_idx, int *stuff_id = NULL );

        void draw(Camera* cam,byte player_mask,bool FLAT=false,float niv=0.0f,float t=0.0f,float dt=1.0f,bool depth_only=false,bool check_visibility=true,bool draw_uw=true);

        Vector3D hit(Vector3D Pos,Vector3D Dir,bool water = true, float length = 200000.0f, bool allow_out = false);			// Calcule l'intersection d'un rayon avec la carte(le rayon partant du dessus de la carte)
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
        Vector3D		*point;
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
            if(point)		delete[] point;
            if(texcoord)	delete[] texcoord;
            if(index)		delete[] index;
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



    class SKY_DATA
    {
    public:
        bool			spherical;			// Flat or spherical sky
        float			rotation_speed;		// For spherical sky
        float			rotation_offset;	// If you want the sun to match light dir ...
        String			texture_name;		// Name of the texture used as sky
        String::Vector	planet;				// Vector of planets that can use this sky
        float			FogColor[4];		// Color of the fog to use with this sky
        String::Vector	MapName;			// Name of maps linked with this sky
        bool			full_sphere;		// The texture is for the whole sphere
        bool			def;

        SKY_DATA();

        ~SKY_DATA();

        void load_tdf( const String	&filename );
    };

    SKY_DATA	*choose_a_sky( const String &mapname, const String &planet );


}

#endif // __TA3D_ENGINE_CL_H__
