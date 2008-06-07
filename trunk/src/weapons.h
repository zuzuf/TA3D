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
|                                       weapons.h                                    |
|   Ce module contient les structures, classes et fonctions nécessaires à la lecture |
| des fichiers tdf du jeu totalannihilation concernant les armes utilisées par les   |
| unités du jeu.                                                                     |
|                                                                                    |
\-----------------------------------------------------------------------------------*/

#include "gaf.h"

#ifndef __WEAPON_CLASSES
#define __WEAPON_CLASSES

#define RENDER_TYPE_LASER		0x0
#define RENDER_TYPE_MISSILE		0x1
#define RENDER_TYPE_GUN			0x2
#define RENDER_TYPE_DGUN		0x3
#define RENDER_TYPE_BITMAP		0x4
#define RENDER_TYPE_PARTICLES	0x5
#define RENDER_TYPE_BOMB		0x6
#define RENDER_TYPE_LIGHTNING	0x7
#define RENDER_TYPE_NONE		0x8

using namespace TA3D;
using namespace TA3D::INTERFACES;

class WEAPON_DEF				// Classe définissant un type d'arme
{
public:
	short	weapon_id;			// Numéro identifiant l'arme
	char	*internal_name;		// Nom interne de l'arme
	char	*name;				// Nom de l'arme
	byte	rendertype;
	bool	ballistic;
	bool	dropped;
	bool	turret;
	int		range;				// portée
	float	reloadtime;			// temps de rechargement
	float	weaponvelocity;
	float	time_to_range;		// Time needed to get to range
	short	areaofeffect;		// zone d'effet
	bool	startsmoke;
	bool	endsmoke;
	int		damage;				// Dégats causés par l'arme
	byte	firestarter;
	int		accuracy;
	int		aimrate;
	int		tolerance;
	short	holdtime;
	int		energypershot;
	int		metalpershot;
	int		minbarrelangle;
	bool	unitsonly;
	float	edgeeffectiveness;
	bool	lineofsight;
	int		color[4];
	short	burst;
	float	burstrate;
	float	duration;
	bool	beamweapon;
	bool	burnblow;
	float	startvelocity;			// Pour les missiles
	float	weapontimer;			// Pour les missiles
	float	weaponacceleration;		// Pour les missiles
	int		turnrate;				// Pour les missiles
	MODEL	*model;					// Modèle 3D
	float	smokedelay;
	bool	guidance;				// Guidage
	bool	tracks;
	bool	selfprop;
	bool	smoketrail;				// Laisse de la fumée en passant
	bool	noautorange;
	bool	noexplode;
	short	flighttime;
	bool	vlaunch;
	bool	stockpile;
	bool	targetable;				// On peut viser
	bool	commandfire;			// ne tire pas seul
	bool	cruise;
	bool	propeller;
	bool	twophase;
	short	shakemagnitude;
	float	shakeduration;
	char	*soundstart;			// Son à jouer lorsque l'arme tire
	char	*soundhit;				// Son à jouer lorsque l'arme explose (si l'arme explose)
	char	*soundwater;			// Son à jouer lorsque l'arme touche l'eau
	char	*soundtrigger;			// Son à jouer lorsque l'arme tire en mode rapide
	char	*explosiongaf;
	char	*explosionart;
	char	*waterexplosiongaf;
	char	*waterexplosionart;
	char	*lavaexplosiongaf;
	char	*lavaexplosionart;
	short	nb_id;
	bool	waterweapon;
	int		metal;
	int		energy;
	bool	interceptor;			// Prend pour cible des armes / Target weapons only
	float	coverage;				// Zone de protection
	bool	toairweapon;			// Only attacks flying units ?
	cHashTable< int >	*damage_hashtable;		// hashtable used to get specific damages quickly

	inline void init()
	{
		damage_hashtable = new cHashTable< int >( 128 );

		soundstart = NULL;
		soundhit = NULL;
		soundwater = NULL;
		soundtrigger = NULL;

		toairweapon = false;
		time_to_range=2.0f;
		burnblow=false;
		coverage=0.0f;
		interceptor=false;
		metal=0;
		energy=0;
		waterweapon=false;
		explosiongaf=NULL;
		explosionart=NULL;
		waterexplosiongaf=NULL;
		waterexplosionart=NULL;
		lavaexplosiongaf=NULL;
		lavaexplosionart=NULL;
		dropped=false;
		nb_id=0;
		burst=1;
		noexplode=false;
		weapon_id=0;			// Numéro identifiant l'arme
		internal_name=NULL;		// Nom interne de l'arme
		name=NULL;				// Nom de l'arme
		rendertype=RENDER_TYPE_NONE;
		ballistic=false;
		turret=false;
		range=100;				// portée
		reloadtime=0.0f;		// temps de rechargement
		weaponvelocity=0.0f;
		areaofeffect=10;		// zone d'effet
		startsmoke=false;
		endsmoke=false;
		damage=100;				// Dégats causés par l'arme
		firestarter=0;
		accuracy=0;
		aimrate=0;
		tolerance=0;
		holdtime=0;
		energypershot=0;
		metalpershot=0;
		minbarrelangle=0;
		unitsonly=false;
		edgeeffectiveness=1.0f;
		lineofsight=false;
		color[0]=0;
		color[1]=0xFFFFFF;
		color[2]=0;
		color[3]=0;
		burstrate=1.0f;
		duration=0.1f;
		beamweapon=false;
		startvelocity=0;			// Pour les missiles
		weapontimer=0.0f;			// Pour les missiles
		weaponacceleration=0.0f;	// Pour les missiles
		turnrate=1;					// Pour les missiles
		model=NULL;					// Modèle 3D
		smokedelay=0.1f;
		guidance=false;				// Guidage
		tracks=false;
		selfprop=false;
		smoketrail=false;			// Laisse de la fumée en passant
		noautorange=false;
		flighttime=10;
		vlaunch=false;
		stockpile=false;			// Nécessite de fabriquer des munitions / need to make ammo??
		targetable=false;			// On peut viser
		commandfire=false;			// ne tire pas seul
		cruise=false;
		propeller=false;
		twophase=false;
		shakemagnitude=0;
		shakeduration=0.1f;
	}

	uint32	get_damage_for_unit( const String &unit_name );

	inline void destroy()
	{
		if(soundstart) 	free(soundstart);
		if(soundhit)		free(soundhit);
		if(soundwater)		free(soundwater);
		if(soundtrigger)	free(soundtrigger);
		if(internal_name)	free(internal_name);
		if(name)	free(name);
		if( damage_hashtable )	delete damage_hashtable;
		init();
	}

	WEAPON_DEF() : damage_hashtable()
	{
		init();
	}

	~WEAPON_DEF()
	{
		destroy();
		if( damage_hashtable )	delete damage_hashtable;
	}
};

class WEAPON_MANAGER			// Gestionnaire de types d'armes
{
public:
	int			nb_weapons;
	WEAPON_DEF	*weapon;
	ANIM		cannonshell;	// Animation pour les tirs de cannons
	cHashTable< int >	weapon_hashtable;		// hashtable used to speed up operations on WEAPON_DEF objects

	void init()
	{
		nb_weapons=0;
		weapon=NULL;
		cannonshell.init();
	}

	WEAPON_MANAGER() : weapon_hashtable()
	{
		init();
	}

	void destroy()
	{
		cannonshell.destroy();
		if(nb_weapons>0 && weapon)			// Détruit les éléments
			for(int i=0;i<nb_weapons;i++)
				weapon[i].destroy();
		if(weapon)
			free(weapon);
		weapon_hashtable.EmptyHashTable();
		weapon_hashtable.InitTable( __DEFAULT_HASH_TABLE_SIZE );
		init();
	}

	~WEAPON_MANAGER()
	{
		destroy();
		weapon_hashtable.EmptyHashTable();
	}

	int add_weapon(char *name)			// Ajoute un élément
	{
		nb_weapons++;
		WEAPON_DEF *n_weapon=(WEAPON_DEF*) malloc(sizeof(WEAPON_DEF)*nb_weapons);
		if(weapon && nb_weapons>1)
			for(int i=0;i<nb_weapons-1;i++)
				n_weapon[i]=weapon[i];
		if(weapon)	free(weapon);
		weapon=n_weapon;
		weapon[nb_weapons-1].init();
		weapon[nb_weapons-1].internal_name=strdup(name);
		weapon[nb_weapons-1].nb_id=nb_weapons-1;

		weapon_hashtable.Insert( Lowercase( name ), nb_weapons );

		return nb_weapons-1;
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

	void load_tdf(char *data,int size=99999999);					// Charge un fichier tdf

	int get_weapon_index(char *name)
	{
		if(name==NULL)	return -1;
		if(nb_weapons<=0)	return -1;
		return weapon_hashtable.Find( Lowercase( name ) ) - 1;
	}
};

extern WEAPON_MANAGER		weapon_manager;

void load_weapons(void (*progress)(float percent,const String &msg)=NULL);				// Charge tout les éléments

class WEAPON						// Objet arme utilisé pendant le jeu
{
public:
	short		weapon_id;			// Identifiant de l'arme
	VECTOR		Pos;				// Position
	VECTOR		V;					// Vitesse
	VECTOR		Ac;					// Accélération
	VECTOR		target_pos;			// Position ciblée
	short		target;				// Unité ciblée (dans le tableau des unités)
	float		stime;				// Temps écoulé depuis le lancement
	float		killtime;			// Temps écoulé depuis la destruction de l'arme
	bool		dying;
	float		smoke_time;			// Temps écoulé depuis la dernière émission de fumée
	float		f_time;				// Temps de vol
	float		a_time;				// Temps d'activité
	short		anim_sprite;		// Position dans l'animation
	short		shooter_idx;		// Unité qui a tiré l'arme (ne peut pas se tirer dessus)
	byte		phase;
	byte		owner;
	uint32		idx;
	bool		visible;
	float		damage;
	bool		just_explode;		// When set the weapon behaves as if it had hit something
	bool		local;
	uint32		last_timestamp;

	inline void init()
	{
		last_timestamp = 0;
		just_explode = false;
		damage = -1;
		visible=true;
		idx=0;
		phase=1;
		a_time=0.0f;
		f_time=0.0f;
		shooter_idx=-1;
		anim_sprite=0;
		weapon_id=-1;		// Non défini
		Pos.x=Pos.y=Pos.z=0.0f;
		Ac=V=Pos;
		target_pos=Pos;
		target=-1;			// Pas de cible
		stime=0.0f;
		killtime=0.0f;
		dying=false;
		smoke_time=0.0f;
		owner=0;
		local = true;
	}

	WEAPON()
	{
		init();
	}

	const void move(const float dt,MAP *map);

	void draw(CAMERA *cam=NULL,MAP *map=NULL);
};

class INGAME_WEAPONS :	protected cCriticalSection,			// The weapon manager
						public cThread						// Inherit things we need to use threads
{
public:
	uint32		nb_weapon;			// Nombre d'armes
	uint32		max_weapon;			// Nombre maximum d'armes stockables dans le tableau
	WEAPON		*weapon;			// Tableau regroupant les armes
	ANIM		nuclogo;			// Logos des armes atomiques sur la minicarte / Logo of nuclear weapons on minimap

	uint32		index_list_size;	// Pre allocated list of used indexes
	uint32		*idx_list;
	uint32		free_index_size;	// Pre allocated list of unused indexes
	uint32		*free_idx;

	protected:
		bool	thread_running;
		bool	thread_ask_to_stop;
		MAP		*p_map;

		int			Run();
		void		SignalExitThread();

	public:

	inline void set_data( MAP *map )	{	p_map = map;	}

	inline void Lock()		{	EnterCS();	}
	inline void UnLock()	{	LeaveCS();	}

	void init(bool real=true)
	{
		EnterCS();

		thread_running = false;
		thread_ask_to_stop = false;

		index_list_size = 0;
		idx_list = NULL;
		free_index_size = 0;
		free_idx = NULL;

		nb_weapon=0;
		max_weapon=0;
		weapon=NULL;
		nuclogo.init();
		if(real) {
			byte *data=HPIManager->PullFromHPI("anims\\fx.gaf");
			if(data) {
				nuclogo.load_gaf(data,get_gaf_entry_index(data,"nuclogo"));
				nuclogo.convert();
				nuclogo.clean();
				free(data);
				}
			}

		LeaveCS();
	}

	void destroy()
	{
		DestroyThread();

		EnterCS();

		if(idx_list)	delete[] idx_list;
		if(free_idx)	delete[] free_idx;
		index_list_size = 0;		idx_list = NULL;
		free_index_size = 0;		free_idx = NULL;
		if(max_weapon>0 && weapon)
			free(weapon);
		nuclogo.destroy();

		LeaveCS();

		init(false);
	}

	INGAME_WEAPONS()
	{
		CreateCS();

		InitThread();

		init(false);
	}

	~INGAME_WEAPONS()
	{
		destroy();

		DeleteCS();
	}

	int add_weapon(int weapon_id,int shooter)
	{
		if(weapon_id<0)	return -1;

		EnterCS();

		if(nb_weapon<max_weapon) {		// S'il y a encore de la place
			uint32 i = free_idx[--free_index_size];
			idx_list[index_list_size++] = i;
			nb_weapon++;
			weapon[i].init();
			weapon[i].weapon_id=weapon_id;
			weapon[i].shooter_idx=shooter;
			weapon[i].idx=i;
			weapon[i].f_time=weapon_manager.weapon[weapon_id].flighttime;
LeaveCS();
			return i;
			}
		else {
			max_weapon+=100;			// Augmente la taille du tableau
			WEAPON *new_weapon=(WEAPON*) malloc(sizeof(WEAPON)*max_weapon);

			uint32	*n_idx = new uint32[max_weapon];
			uint32	*n_new_idx = new uint32[max_weapon];
			if(index_list_size>0)
				memcpy(n_idx,idx_list,index_list_size<<2);
			if(free_index_size>0)
				memcpy(n_new_idx,free_idx,free_index_size<<2);
			if(idx_list)	delete[]	idx_list;
			if(free_idx)	delete[]	free_idx;
			idx_list = n_idx;
			free_idx = n_new_idx;
			for(uint32 i = max_weapon-100; i<max_weapon;i++)
				free_idx[free_index_size++] = i;

			for(uint32 i=0;i<max_weapon;i++)
				new_weapon[i].weapon_id=-1;
			if(weapon && nb_weapon>0)
				for(uint32 i=0;i<nb_weapon;i++)
					new_weapon[i]=weapon[i];
			if(weapon)
				free(weapon);
			weapon=new_weapon;
			uint32 index = free_idx[--free_index_size];
			idx_list[index_list_size++] = index;
			nb_weapon++;
			weapon[index].init();
			weapon[index].weapon_id=weapon_id;
			weapon[index].shooter_idx=shooter;
			weapon[index].idx=index;
			weapon[index].f_time=weapon_manager.weapon[weapon_id].flighttime;
LeaveCS();
			return index;
			}
	}

	void move(float dt,MAP *map)
	{
		if(nb_weapon<=0 || max_weapon<=0) {	rest( 1 );	return;	}

		EnterCS();

		for(uint32 e=0;e<index_list_size;e++) {
			LeaveCS();						// Pause to give the renderer the time to work and to go at the given engine speed (in ticks per sec.)
			EnterCS();

			uint32 i = idx_list[e];
			weapon[i].move(dt,map);
			if(weapon[i].weapon_id<0) {		// Remove it from the "alive" list
				nb_weapon--;
				free_idx[free_index_size++] = i;
				idx_list[e--] = idx_list[--index_list_size];
				}
			}

		LeaveCS();
	}

	void draw(CAMERA *cam=NULL,MAP *map=NULL,bool underwater=false)
	{
		if(nb_weapon<=0 || max_weapon<=0) return;

		EnterCS();
		gfx->GFX_EnterCS();

		if( cam )
			cam->SetView();

		for(uint32 e=0;e<index_list_size;e++) {
			uint32 i = idx_list[e];
			if((weapon[i].Pos.y<map->sealvl && underwater) || (weapon[i].Pos.y>=map->sealvl && !underwater))
				weapon[i].draw(cam,map);
			}

		gfx->GFX_LeaveCS();
		LeaveCS();
	}

	inline void draw_mini(float map_w,float map_h,int mini_w,int mini_h)				// Repère les unités sur la mini-carte
	{
		if(nb_weapon<=0 || max_weapon<=0)	return;		// Pas d'unités à dessiner

		EnterCS();

		float rw=128.0f*mini_w/252;
		float rh=128.0f*mini_h/252;

		glColor4f(1.0f,1.0f,1.0f,1.0f);
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_POINTS);
		for(uint32 e=0;e<index_list_size;e++) {
			uint32 i = idx_list[e];
			if(weapon_manager.weapon[weapon[i].weapon_id].cruise || weapon_manager.weapon[weapon[i].weapon_id].interceptor) {
				glEnd();
				glEnable(GL_TEXTURE_2D);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
				int idx=weapon[i].owner;
				PutTex(nuclogo.glbmp[idx],weapon[i].Pos.x/map_w*rw+64.0f-nuclogo.ofs_x[idx],weapon[i].Pos.z/map_h*rh+64.0f-nuclogo.ofs_y[idx],weapon[i].Pos.x/map_w*rw+63.0f-nuclogo.ofs_x[idx]+nuclogo.w[idx],weapon[i].Pos.z/map_h*rh+63.0f-nuclogo.ofs_y[idx]+nuclogo.h[idx]);
				glDisable(GL_BLEND);
				glDisable(GL_TEXTURE_2D);
				glBegin(GL_POINTS);
				}
			else
				glVertex2f(weapon[i].Pos.x/map_w*rw+64.0f,weapon[i].Pos.z/map_h*rh+64.0f);
			}
		glEnd();
		glEnable(GL_TEXTURE_2D);
LeaveCS();
	}
};

extern INGAME_WEAPONS weapons;

class FX
{
public:
	float		time;		// Durée d'éxistence de l'effet
	bool		playing;	// Si l'effet est joué
	VECTOR		Pos;		// Position
	float		size;		// Taille (proportion de l'image d'origine)
	int			anm;		// Animation

public:
	inline void init()
	{
		time=0.0f;
		playing=false;
		Pos.x=Pos.y=Pos.z=0.0f;
		size=1.0f;
		anm=0;
	}

	inline void destroy()
	{
		init();
	}

	FX()
	{
		init();
	}

	~FX()
	{
		destroy();
	}

	inline bool move(float dt,ANIM **anims)
	{
		if(!playing)	return false;
		if(anm == -1) {							// Flash effect
			if( time > 1.0f )	{
				playing=false;
				return true;
				}
			time+=dt;
			return false;
			}
		if( anm == -2 || anm == -3 || anm == -4 || anm == -5 ) {							// Wave effect on shores or ripple
			if( time > 4.0f || ( time > 2.0f && anm == -5 ) )	{
				playing=false;
				return true;
				}
			time+=dt;
			return false;
			}
		if(anm<0) {
			playing=false;
			return false;
			}
		time+=dt;
		if(time*15.0f>=anims[anm]->nb_bmp) {
			playing=false;
			return true;
			}
		return false;
	}

	void draw(CAMERA *cam, MAP *map, ANIM **anims);

	inline void load(int anim,VECTOR P,float s)
	{
		destroy();

		anm=anim;
		Pos=P;
		size=s*0.25f;
		time=0.0f;
		playing=true;
	}
};

class FX_MANAGER : cCriticalSection				// This class mustn't be executed in its own thread in order to remain thread safe,
{												// it must run in main thread (the one that can call OpenGL functions)!!
private:
	int			max_fx;
	int			nb_fx;
	FX			*fx;

	int			cache_size;			// Cache
	int			max_cache_size;
	char		**cache_name;
	ANIM		**cache_anm;
	int			*use;
	bool		cache_is_dirty;

public:
	byte		*fx_data;
	GLuint		flash_tex;
	GLuint		wave_tex[3];
	GLuint		ripple_tex;

	inline void init()
	{
		cache_is_dirty = false;

		fx_data=NULL;

		max_fx=0;
		nb_fx=0;
		fx=NULL;

		max_cache_size=0;
		cache_size=0;
		cache_name=NULL;
		cache_anm=NULL;
		use=NULL;

		flash_tex = 0;
		wave_tex[0] = 0;
		wave_tex[1] = 0;
		wave_tex[2] = 0;
		ripple_tex = 0;
	}

	inline void destroy()
	{
		gfx->destroy_texture( flash_tex );
		gfx->destroy_texture( ripple_tex );
		gfx->destroy_texture( wave_tex[0] );
		gfx->destroy_texture( wave_tex[1] );
		gfx->destroy_texture( wave_tex[2] );

		if(fx_data)	free(fx_data);
		if(fx) {
			for(int i=0;i<max_fx;i++)
				fx[i].destroy();
			free(fx);
			}
		if(cache_size>0) {
			if(cache_name) {
				for(int i=0;i<max_cache_size;i++)
					if(cache_name[i])
						free(cache_name[i]);
				free(cache_name);
				}
			if(cache_anm) {
				for(int i=0;i<max_cache_size;i++)
					if(cache_anm[i]) {
						cache_anm[i]->destroy();
						delete cache_anm[i];
						}
				free(cache_anm);
				}
			}
		init();
	}

	FX_MANAGER()
	{
		CreateCS();

		init();
	}

	~FX_MANAGER()
	{
		destroy();

		DeleteCS();
	}

	void load_data();

	inline int is_in_cache(char *filename)
	{
		if(cache_size<=0)	return -1;
		for(int i=0;i<max_cache_size;i++)
			if(cache_anm[i]!=NULL && cache_name[i]!=NULL)
				if(strcasecmp(filename,cache_name[i])==0)
					return i;
		return -1;
	}

	inline int put_in_cache(char *filename,ANIM *anm)
	{
		EnterCS();

		int is_in=is_in_cache(filename);
		if(is_in>=0)	return is_in;		// On ne le garde pas 2 fois
		int idx=-1;
		if(cache_size+1>max_cache_size) {
			max_cache_size+=100;
			char **n_name=(char**)malloc(sizeof(char*)*max_cache_size);
			ANIM **n_anm=(ANIM**)malloc(sizeof(ANIM*)*max_cache_size);
			int *n_use=(int*)malloc(sizeof(int)*max_cache_size);
			for(int i=max_cache_size-100;i<max_cache_size;i++) {
				n_use[i]=0;
				n_name[i]=NULL;
				n_anm[i]=NULL;
				}
			if(cache_size>0) {
				for(int i=0;i<max_cache_size-100;i++) {
					n_name[i]=cache_name[i];
					n_anm[i]=cache_anm[i];
					n_use[i]=use[i];
					}
				free(cache_name);
				free(cache_anm);
				free(use);
				}
			use=n_use;
			cache_name=n_name;
			cache_anm=n_anm;
			idx=cache_size;
			}
		else {
			idx=0;
			for(int i=0;i<max_cache_size;i++)
				if(cache_anm[i]==NULL)
					idx=i;
			}
		use[idx]=1;
		cache_anm[idx]=anm;
		cache_name[idx]=strdup(filename);
		cache_size++;

		LeaveCS();

		return idx;
	}

	inline void move(float dt)
	{
		EnterCS();

		for(int i=0;i<max_fx;i++)
			if(fx[i].move(dt,cache_anm)) {
				if( fx[i].anm == -1 || fx[i].anm == -2 || fx[i].anm == -3 || fx[i].anm == -4 || fx[i].anm == -5 )	{
					nb_fx--;
					continue;		// Flash, ripple or Wave
					}
				use[fx[i].anm]--;
				nb_fx--;
				if(use[fx[i].anm]<=0) {		// Animation inutilisée
					free(cache_name[fx[i].anm]);		cache_name[fx[i].anm]=NULL;
					cache_anm[fx[i].anm]->destroy();	delete cache_anm[fx[i].anm];	cache_anm[fx[i].anm]=NULL;
					cache_size--;
					}
				}

		LeaveCS();
	}

	inline void draw(CAMERA *cam, MAP *map, float w_lvl=0.0f, bool UW=false)
	{
		EnterCS();

		if( cache_is_dirty ) {			// We have work to do
			for( uint32 i = 0 ; i < max_cache_size ; i++ )
				if( cache_anm[i] )
					cache_anm[i]->convert(false,true);
			cache_is_dirty = false;
			}

		glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

		glEnable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		cam->SetView();
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0.0f,-1600.0f);
		if(UW) {
			for(int i=0;i<max_fx;i++)
				if( fx[i].playing && fx[i].Pos.y<w_lvl )
					fx[i].draw(cam,map,cache_anm);
			}
		else
			for(int i=0;i<max_fx;i++)
				if( fx[i].playing && fx[i].Pos.y>=w_lvl )
					fx[i].draw(cam,map,cache_anm);
		glDisable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0.0f,0.0f);
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);

		LeaveCS();
	}

	int add(char *filename,char *entry_name,VECTOR Pos,float size);
	int add_flash(VECTOR Pos,float size);
	int add_wave(VECTOR Pos,float size);
	int add_ripple(VECTOR Pos,float size);
};

extern FX_MANAGER	fx_manager;
#endif
