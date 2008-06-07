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
|                                      particles.h                                   |
|  Ce fichier contient les structures, classes et fonctions nécessaires aux effets   |
| graphiques utilisants des particules comme la poussière, les effets de feu,        |
| d'explosion, de fumée ... Ce fichier est conçu pour fonctionner avec la librairie  |
| Allegro et l'addon AllegroGL pour l'utilisation d'OpenGl avec Allegro.             |
|                                                                                    |
\-----------------------------------------------------------------------------------*/

#ifndef __CLASSE_PARTICLES
#define __CLASSE_PARTICLES

#include "cThread.h"

class PARTICLE_SYSTEM			// Class defining the fast particle engine
{
public:
	uint32			nb_particles;
	VECTOR			*pos;
	VECTOR			*V;
	VECTOR			common_pos;
	VECTOR			common_V;
	float			size;
	float			dsize;
	float			mass;
	float			life;
	float			col[4];
	float			dcol[4];
	bool			use_wind;
	bool			light_emitter;
	GLuint			tex;
	uint16			cur_idx;			// Used to fill the point array

	PARTICLE_SYSTEM() : common_pos(), common_V()
	{
		nb_particles = 0;
		pos = NULL;
		V = NULL;
		life = 1.0f;
		size = 1.0f;
		dsize = 0.0f;
		mass = 1.0f;
		use_wind = true;
		light_emitter = false;
		tex = 0;
		cur_idx = 0;
	}

	~PARTICLE_SYSTEM()	{	destroy();	}

	inline void destroy()
	{
		if( pos )
			delete[]	pos;
		if( V )
			delete[]	V;
		pos = NULL;
		V = NULL;
	}

	void create( uint16 nb, GLuint gltex );

	void move( const float &dt, VECTOR *p_wind_dir, float g, float factor, float factor2 );

	void draw();
};

struct PARTICLE					// Structure définissant une particule
{
	VECTOR		Pos;			// Position
	VECTOR		V;				// Vitesse
	float		size;			// Taille
	float		dsize;			// Variation de la taille au cours du temps
	float		ddsize;			// Variation de dsize au cours du temps
	float		col[4];			// Couleur
	float		dcol[4];		// Variation de couleur
	GLuint		gltex;			// Texture associée
	float		mass;			// Masse apparente de la particule si celle-ci est soumise à la force de gravitation(signe seulement)
	float		life;			// Temps d'existence restant à la particule
	float		smoking;		// Produit de la fumée?
	float		angle;			// Angle
	float		v_rot;			// Vitesse de rotation
	short		px;				// Coordonnées du bloc de référence pour la visibilité
	short		py;
	bool		use_wind;		// Affected by wind ?
	bool		light_emitter;	// For fire effects
	bool		slow_down;		// Decrease speed ?
	float		slow_factor;
};

class PARTICLE_ENGINE : protected cCriticalSection,			// Moteur à particules
						public	cThread
{
public:
	uint32		nb_part;		// Nombre de particules
	uint32		size;			// Quantité maximale de particules stockables dans le tableau
	PARTICLE	*part;			// Tableau de particules
	GLuint		parttex;		// Textures des particules
	BITMAP		*partbmp;		// Textures des particules
	bool		dsmoke;
	uint32		ntex;
	Vector< GLuint >	gltex;

	uint32		index_list_size;	// Pre allocated list of used indexes
	uint32		*idx_list;
	uint32		free_index_size;	// Pre allocated list of unused indexes
	uint32		*free_idx;

	VECTOR		*point;			// Vertex array
	GLfloat		*texcoord;		// Texture coordinates array
	GLubyte		*color;			// Color array

	protected:
		bool	thread_running;
		bool	thread_ask_to_stop;
		VECTOR	*p_wind_dir;
		float	*p_g;

		List< PARTICLE_SYSTEM* >		particle_systems;

		int			Run();
		void		SignalExitThread();

	public:

	inline void set_data( float &g, VECTOR &wind_dir )	{	p_g = &g;	p_wind_dir = &wind_dir;	}

	void init(bool load=true);

	int addtex(const char *file,const char *filealpha);

	PARTICLE_ENGINE() : particle_systems(), gltex()
	{
		CreateCS();

		InitThread();

		init(false);
	}

	void destroy();

	~PARTICLE_ENGINE()
	{
		destroy();

		DeleteCS();
	}

	void more_memory();			// Alloue de la mémoire supplémentaire

	void emit_part(POINTF pos,VECTOR Dir,int tex,int nb,float speed,float life=10.0f,float psize=10.0f,bool white=false,float trans_factor=1.0f);

	PARTICLE_SYSTEM *emit_part_fast( PARTICLE_SYSTEM *system, POINTF pos, VECTOR Dir, int tex, int nb, float speed, float life=10.0f, float psize=10.0f, bool white=false, float trans_factor=1.0f );

	void emit_lava(POINTF pos,VECTOR Dir,int tex,int nb,float speed,float life=10.0f);

	void make_smoke(POINTF pos,int tex,int nb,float speed,float mass=-1.0f, float ddsize=0.0f,float alpha=1.0f);

	void make_fire(POINTF pos,int tex,int nb,float speed);

	void make_shockwave(POINTF pos,int tex,int nb,float speed);

	void make_nuke(POINTF pos,int tex,int nb,float speed);

	void move(float dt,VECTOR wind_dir,float g=9.81f);

	void draw(CAMERA *cam,int map_w,int map_h,int bloc_w,int bloc_h,byte **bmap);
};

extern PARTICLE_ENGINE	particle_engine;

#endif
