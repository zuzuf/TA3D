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
|                                         fx.h                                       |
|   This modules contains the special FX engine, it draws sprites, waves, light      |
| halos, bouncing particles, ...                                                     |
|                                                                                    |
\-----------------------------------------------------------------------------------*/

#ifndef __FX_H_
#define __FX_H_

#include "gaf.h"

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

class FX_PARTICLE
{
	private:
		VECTOR	Pos, Speed;
		float	timer;
		float	life;
		
	public:
		FX_PARTICLE( VECTOR &P, VECTOR &S, float L );
		bool move( float dt );
		void draw();
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
	
	List< FX_PARTICLE >	particles;			// List of particles bouncing around

public:
	byte		*fx_data;
	GLuint		flash_tex;
	GLuint		wave_tex[3];
	GLuint		ripple_tex;

	void init();
	void destroy();

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
				if(use[fx[i].anm]<=0) {		// Animation used nowhere
					free(cache_name[fx[i].anm]);		cache_name[fx[i].anm]=NULL;
					cache_anm[fx[i].anm]->destroy();	delete cache_anm[fx[i].anm];	cache_anm[fx[i].anm]=NULL;
					cache_size--;
					}
				}

		foreach_( particles, i )
			if( i->move( dt ) )
				particles.erase( i++ );
			else
				i++;

		LeaveCS();
	}

	void draw(CAMERA *cam, MAP *map, float w_lvl=0.0f, bool UW=false);
	int add(char *filename,char *entry_name,VECTOR Pos,float size);
	int add_flash(VECTOR Pos,float size);
	int add_wave(VECTOR Pos,float size);
	int add_ripple(VECTOR Pos,float size);
};

extern FX_MANAGER	fx_manager;

#endif
