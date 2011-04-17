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
# include "misc/string.h"
# include "misc/grid.h"


#define PARTICLE_LIMIT		100000		// pas plus de 100000 particules
#define HMAP_RESIZE			0.04f

#define H_DIV		0.5f


namespace TA3D
{

// tnt_transform = 1.0f / tanf(63.44f * DEG2RAD) / H_DIV;
#define tnt_transform			0.99977961f
// tnt_transform_H_DIV = 1.0f / tanf(63.44f * DEG2RAD);
#define tnt_transform_H_DIV		0.49988981f

	class SECTOR			// Structure pour regrouper les informations sur le terrain (variations d'altitude, submergé, teneur en metal, ...)
	{
	public:
		sint32			stuff;				// Indique l'élément graphique présent sur ce secteur
		sint32			unit_idx;			// Indice de l'unité qui se trouve sur ce secteur
		uint32			flags;				// underwater, lava, flat

		inline bool isUnderwater()	const	{	return flags & 1U;	}	// is the bloc under water ?
		inline bool isLava()		const	{	return flags & 2U;	}	// is the bloc under lava ? used for pathfinding
		inline bool isFlat()		const	{	return flags & 4U;	}	// is the bloc flat ? used in the renderer to simplify geometry

		inline void setUnderwater()		{	flags |= 1U;	}
		inline void setLava()			{	flags |= 2U;	}
		inline void setFlat()			{	flags |= 4U;	}
		inline void unsetUnderwater()	{	flags &= ~1U;	}
		inline void unsetLava()			{	flags &= ~2U;	}
		inline void unsetFlat()			{	flags &= ~4U;	}

		inline void setUnderwater(bool b)	{	flags = b ? (flags | 1U) : (flags & ~1U);	}
		inline void setLava(bool b)			{	flags = b ? (flags | 2U) : (flags & ~2U);	}
		inline void setFlat(bool b)			{	flags = b ? (flags | 4U) : (flags & ~4U);	}

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

		void load(File *data);
		void load(const String& filename);

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
		bool	whitefog;

	}; // class MAP_OTA



	class BLOC				// Blocs composant la carte
	{
	public:
		Vector3D	*point;		// Points du bloc / Array of points
		float		*texcoord;	// Coordonnées de texture / Texture coordinates
		GLuint		tex;		// Indice de texture OpenGl / OpenGL texture handle
		byte		nbindex;	// Nombre d'indices	/ Number of indexes
		byte		nbpoint;	// Nombre de points / Number of points
		bool		lava;		// Indique si le bloc est de type lave / Is that a lava bloc ?
		byte		tex_x;

		void init()
		{
			nbindex = nbpoint = 0;
			point = NULL;
			texcoord = NULL;
			tex = 0;
			lava = false;
			tex_x = 0;
		}

		BLOC()
		{
			init();
		}

		void destroy()
		{
			DELETE_ARRAY(point);
			DELETE_ARRAY(texcoord);
			init();
		}
	};

	class MAP : public ObjectSync // Données concernant la carte
	{
	public:
		short		ntex;			// Indique si la texture est chargée et doit être détruite
		GLuint		*tex;			// Texture de surface
		int			nbbloc;			// Nombre de blocs
		BLOC		*bloc;			// Blocs composant le terrain
		Grid<uint16>	bmap;		// Tableau d'indice des blocs
		Grid<float>	h_map;		// Tableau de l'élévation du terrain
		Grid<float>	ph_map;		// Tableau du relief projeté pour le calcul inverse(projection) lors de l'affichage
		Grid<SECTOR>	map_data;		// Tableau d'informations sur le terrain
		Grid<byte>	view;			// Indique quels sont les parcelles de terrain visibles à l'écran
		Grid<int>	path;			// Tableau pour le pathfinding
		Grid<float> slope;			// Maximum derivative of the height map
		Grid<float>	energy;			// Energy of the map used by the pathfinder and units when following a path
		Grid<bool>	obstacles;		// A map used by the pathfinder to detect blocking objets

		Grid<byte> view_map;		// Map of what has been discovered
		Grid<byte> sight_map;		// Map of who is viewing
		Grid<byte> radar_map;		// Radar map
		Grid<byte> sonar_map;		// Sonar map

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
		SDL_Surface *mini;			// Minimap
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
		Shader		shadow2_shader;	// pixel shader to use the shadow map in light equation(also add the detail texture correctly)

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

		std::vector<Vector3D> get_visible_volume() const;

		void drawCircleOnMap(const float x, const float y, const float radius, const uint32 color, const float thickness = 5.0f) const;

		void update_player_visibility( int player_id, int px, int py, int r, int rd, int sn, int rd_j, int sn_j, bool jamming=false, bool black=false );	// r -> sight, rd -> radar range, sn -> sonar range, j for jamming ray

		void draw_mini(int x1=0,int y1=0,int w=252,int h=252, Camera *cam=NULL, byte player_mask=0xFF ); // Dessine la mini-carte

		float get_unit_h(float x, float y) const;

		inline float get_h(const int x, const int y) const;

		float get_max_h(int x,int y) const;

		float get_max_rect_h(int x,int y, int w, int h) const;

		inline float get_zdec(const int x, const int y) const;

		inline int get_zdec_notest(const int x, const int y) const;

		inline float get_nh(const int x, const int y) const;

		void rect(int x1,int y1,int w,int h,int c,const String &yardmap = String(),bool open = false);
		void obstaclesRect(int x1,int y1,int w,int h, bool b,const String &yardmap = String(),bool open = false);

		bool check_rect(int x1,int y1,int w,int h, const int c) const;

		bool check_rect_discovered(int x1,int y1,int w,int h,const int c) const; // Check if the area has been fully discovered

		float check_rect_dh(int x1,int y1,int w,int h) const;

		float check_max_depth(int x1,int y1,int w,int h) const;

		float check_min_depth(int x1,int y1,int w,int h) const;

		bool check_vents(int x1,int y1,int w,int h,const String &yard_map) const;

		bool check_lava(int x1,int y1,int w,int h) const;

		int check_metal(int x1, int y1, int unit_idx, int *stuff_id = NULL) const;

		void draw_LD(byte player_mask,bool FLAT = false, float niv=0.f, float t = 0.f);
		void draw_HD(Camera* cam,byte player_mask,bool FLAT=false,float niv=0.0f,float t=0.0f,float dt=1.0f,bool depth_only=false,bool check_visibility=true,bool draw_uw=true);
		void draw(Camera* cam,byte player_mask,bool FLAT=false,float niv=0.0f,float t=0.0f,float dt=1.0f,bool depth_only=false,bool check_visibility=true,bool draw_uw=true);

		Vector3D hit(Vector3D Pos,Vector3D Dir,bool water = true, float length = 200000.0f, bool allow_out = false) const;			// Calcule l'intersection d'un rayon avec la carte(le rayon partant du dessus de la carte)
	};

	inline float MAP::get_nh(const int x, const int y) const	{	return ph_map(Math::Clamp(x, 0, bloc_w_db - 2), Math::Clamp(y, 0, bloc_h_db - 2));	}
	inline float MAP::get_zdec(const int x, const int y) const	{	return ph_map(Math::Clamp(x, 0, bloc_w_db - 2), Math::Clamp(y, 0, bloc_h_db - 2)) * tnt_transform_H_DIV;	}
	inline int MAP::get_zdec_notest(const int x, const int y) const	{	return 	static_cast<int>(ph_map(x, y) * (0.125f * tnt_transform_H_DIV) + 0.5f);	}
	inline float MAP::get_h(const int x, const int y) const	{	return h_map(Math::Clamp(x, 0, bloc_w_db - 2), Math::Clamp(y, 0, bloc_h_db - 2));	}

	extern MAP *the_map;



	class WATER
	{
	public:
		typedef SmartPtr<WATER>	Ptr;
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

		void draw(float t, bool shaded = false);
	};
}

#endif // __TA3D_ENGINE_CL_H__
