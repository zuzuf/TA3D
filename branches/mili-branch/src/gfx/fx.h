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

#include "../gaf.h"
#include "../threads/thread.h"

class FX
{
public:
    FX();
    ~FX();

    void init();

    void destroy();

    bool move(float dt,ANIM **anims);
    void draw(CAMERA *cam, MAP *map, ANIM **anims);

    void load(int anim,VECTOR P,float s);

public:
    //! Effect duration
    float time;
    //! Get if the effect has been played
    bool playing;
    //! Position
    VECTOR Pos;
    float size;		// Taille (proportion de l'image d'origine)
    int anm;		// Animation

}; // class FX


class FX_PARTICLE
{
public:
    FX_PARTICLE(const VECTOR &P, const VECTOR &S, const float L );
    bool move( float dt );
    void draw();

private:
    VECTOR Pos;
    VECTOR Speed;
    float timer;
    float life;

};

class FX_MANAGER : public ObjectSync	// This class mustn't be executed in its own thread in order to remain thread safe,
{												// it must run in main thread (the one that can call OpenGL functions)!!
public:
    FX_MANAGER();
    ~FX_MANAGER();


    void init();
    void destroy();
    void load_data();

    int FX_MANAGER::is_in_cache(char *filename);
    int put_in_cache(char *filename,ANIM *anm);
    void move(const float dt);
    void draw(CAMERA *cam, MAP *map, float w_lvl=0.0f, bool UW=false);
    int add(char *filename,char *entry_name,VECTOR Pos,float size);
    int add_flash(VECTOR Pos,float size);
    int add_wave(VECTOR Pos,float size);
    int add_ripple(VECTOR Pos,float size);

    /*!
    ** \brief Add a particle
    ** \param p
    ** \param s
    ** \param l
    */
    void addParticle(const VECTOR &p, const VECTOR &s, const float l);

    /*!
    ** \brief Add an explosion effect
    ** \param p
    ** \param n
    ** \param power
    */
    void addExplosion(const  VECTOR &p, const int n, const float power);


public:
    byte		*fx_data;
    GLuint		flash_tex;
    GLuint		wave_tex[3];
    GLuint		ripple_tex;

private:
    int			max_fx;
    int			nb_fx;
    FX			*fx;

    int			cache_size;			// Cache
    int			max_cache_size;
    char		**cache_name;
    ANIM		**cache_anm;
    int			*use;
    bool pCacheIsDirty;

    List<FX_PARTICLE> particles;			// List of particles bouncing around

}; // class FX_MANAGER


extern FX_MANAGER	fx_manager;

#endif
