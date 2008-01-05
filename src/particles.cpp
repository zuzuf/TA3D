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
|                                      particles.cpp                                 |
|  Ce fichier contient les structures, classes et fonctions nécessaires aux effets   |
| graphiques utilisants des particules comme la poussière, les effets de feu,        |
| d'explosion, de fumée ... Ce fichier est conçu pour fonctionner avec la librairie  |
| Allegro et l'addon AllegroGL pour l'utilisation d'OpenGl avec Allegro.             |
|                                                                                    |
\-----------------------------------------------------------------------------------*/

#ifdef CWDEBUG
#include <libcwd/sys.h>
#include <libcwd/debug.h>
#endif
#include "stdafx.h"
#include "matrix.h"
#include "TA3D_NameSpace.h"
#include "ta3dbase.h"
#include "particles.h"

PARTICLE_ENGINE	particle_engine;

void PARTICLE_SYSTEM::create( uint16 nb, GLuint gltex )
{
	nb_particles = nb;
	index = new GLushort[ nb ];
	pos = new VECTOR[ nb ];
	V = new VECTOR[ nb ];
	common_V.x = common_V.y = common_V.z = 0.0f;
	common_pos = common_pos;
	tex = gltex;

	cur_idx = 0;

	for( int i = 0 ; i < nb ; i++ )
		index[ i ] = i;
}

void PARTICLE_SYSTEM::move( const float &dt, VECTOR *p_wind_dir, float g, float factor, float factor2 )
{
	life -= dt;
	size += dt * dsize;

	common_V = (*p_wind_dir);				// To simplify calculations
	common_V.y -= mass * g;

	common_pos = dt * common_V + common_pos;

	col[0] += dt * dcol[0];
	col[1] += dt * dcol[1];
	col[2] += dt * dcol[2];
	col[3] += dt * dcol[3];

	float real_factor = mass > 0.0f ? factor : factor2;

	for( uint32 i = 0 ; i < nb_particles ; i++ ) {
		V[ i ] = real_factor * V[ i ];
		pos[ i ] = dt * V[ i ] + pos[ i ];
		}
}

void PARTICLE_SYSTEM::draw()
{
	glPushMatrix();

	glColor4fv( col );
	glPointSize( size );
	
	glTranslatef( common_pos.x, common_pos.y, common_pos.z );

	glBindTexture(GL_TEXTURE_2D,tex);

	glVertexPointer( 3, GL_FLOAT, 0, pos);

	if( index != NULL )
		glDrawElements(GL_POINTS, nb_particles, GL_UNSIGNED_SHORT, index);		// draw particles

	glPopMatrix();
}

	int PARTICLE_ENGINE::addtex(char *file,char *filealpha)
	{
		EnterCS();

		dsmoke=true;
		if(partbmp==NULL)
			partbmp=create_bitmap_ex(32,256,256);
		BITMAP *bmp;
		if(filealpha)
			bmp=LoadMaskedTexBmp(file,filealpha);		// Avec canal alpha séparé
		else
			bmp=load_bitmap(file,NULL);					// Avec canal alpha intégré ou Sans canal alpha

		gltex.push_back( gfx->make_texture( bmp ) );

		stretch_blit(bmp,partbmp,0,0,bmp->w,bmp->h,64*(ntex&3),64*(ntex>>2),64,64);
		ntex++;
		destroy_bitmap(bmp);
		if(ntex>1)
			glDeleteTextures(1,&parttex);
		allegro_gl_use_alpha_channel(true);
		allegro_gl_set_texture_format(GL_RGBA8);
		parttex = gfx->make_texture(partbmp, FILTER_TRILINEAR);
		allegro_gl_use_alpha_channel(false);

		LeaveCS();

		return (ntex-1);
	}

	void PARTICLE_ENGINE::more_memory()			// Alloue de la mémoire supplémentaire
	{
		EnterCS();

#define MEMORY_STEP		10000
		size += MEMORY_STEP;
		PARTICLE *tmp=(PARTICLE*) malloc(sizeof(PARTICLE)*size);
		if(nb_part>0 && part)
			memcpy(tmp,part,sizeof(PARTICLE)*nb_part);
		uint32	*n_idx = new uint32[size];
		uint32	*n_new_idx = new uint32[size];
		if(index_list_size>0)
			memcpy(n_idx,idx_list,index_list_size<<2);
		if(free_index_size>0)
			memcpy(n_new_idx,free_idx,free_index_size<<2);
		if(idx_list)	delete[]	idx_list;
		if(free_idx)	delete[]	free_idx;
		idx_list = n_idx;
		free_idx = n_new_idx;
		for(uint32 i = size-MEMORY_STEP; i<size;i++)
			free_idx[free_index_size++] = i;
		if(part)
			free(part);
		part=tmp;

		if( point == NULL )
			point=(VECTOR*) malloc(sizeof(VECTOR)*4000);
		if( texcoord == NULL )
			texcoord=(GLfloat*) malloc(sizeof(GLfloat)*8000);
		if( color == NULL )
			color=(GLubyte*) malloc(sizeof(GLubyte)*16000);
		if( index == NULL ) {
			index=(GLushort*) malloc(sizeof(GLushort)*4000);
			for(uint32 i=0;i<4000;i++)
				index[i]=i;
			}
		LeaveCS();
	}

	void PARTICLE_ENGINE::emit_part(POINTF pos,VECTOR Dir,int tex,int nb,float speed,float life,float psize,bool white,float trans_factor)
	{
		if( !lp_CONFIG->particle )	return;		// If particles are OFF don't add particles
		POINTF O;
		O.x=O.y=O.z=0.0f;
		if(game_cam!=NULL && ((VECTOR)(game_cam->Pos-(O>>pos))).Sq()>=game_cam->zfar2)	return;
		EnterCS();
		while(nb_part+nb>size)			// Si besoin alloue de la mémoire
			more_memory();

		for(int i=0;i<nb;i++) {
			if( !free_index_size )	break;

			uint32	cur_part = free_idx[--free_index_size];
			idx_list[index_list_size++] = cur_part;
			part[cur_part].px=-1;
			part[cur_part].Pos=O>>pos;
			part[cur_part].V=speed*Dir;
			part[cur_part].life=life;
			part[cur_part].mass=0.0f;
			part[cur_part].smoking=-1.0f;
			part[cur_part].gltex=tex;
			if(white) {
				part[cur_part].col[0]=1.0f;
				part[cur_part].col[1]=1.0f;
				part[cur_part].col[2]=1.0f;
				part[cur_part].col[3]=trans_factor;
				}
			else {
				part[cur_part].col[0]=0.8f;
				part[cur_part].col[1]=0.8f;
				part[cur_part].col[2]=1.0f;
				part[cur_part].col[3]=trans_factor;
				}
			part[cur_part].dcol[0]=0.0f;
			part[cur_part].dcol[1]=0.0f;
			part[cur_part].dcol[2]=0.0f;
			if(life>0.0f)
				part[cur_part].dcol[3]=-trans_factor/life;
			else
				part[cur_part].dcol[3]=-0.1f;
			part[cur_part].angle=0.0f;
			part[cur_part].v_rot=(rand_from_table()%200)*0.01f-0.1f;
			part[cur_part].size=psize;
			part[cur_part].use_wind=false;
			part[cur_part].dsize=0.0f;
			part[cur_part].ddsize=0.0f;
			part[cur_part].light_emitter=false;
			part[cur_part].slow_down=false;
			nb_part++;
			}
		LeaveCS();
	}

	PARTICLE_SYSTEM *PARTICLE_ENGINE::emit_part_fast( PARTICLE_SYSTEM *system, POINTF pos, VECTOR Dir, int tex, int nb, float speed, float life, float psize, bool white, float trans_factor )
	{
		if( !lp_CONFIG->particle )	return NULL;		// If particles are OFF don't add particles
		if( tex < 0 || tex >= gltex.size() )	return NULL;		// We don't have that texture !!
		POINTF O;
		O.x=O.y=O.z=0.0f;

		if( system ) {			// Step by step
			system->pos[ system->cur_idx ] = O>>pos;
			system->V[ system->cur_idx ] = speed * Dir;
			system->cur_idx++;
			}
		else {

			if(game_cam!=NULL && ((VECTOR)(game_cam->Pos-(O>>pos))).Sq()>=game_cam->zfar2)	return NULL;

			system = new PARTICLE_SYSTEM;
			system->create( abs( nb ), gltex[ tex ] );

			system->life = life;
			system->mass = 0.0f;
			if(white) {
				system->col[0] = 1.0f;
				system->col[1] = 1.0f;
				system->col[2] = 1.0f;
				system->col[3] = trans_factor;
				}
			else {
				system->col[0] = 0.8f;
				system->col[1] = 0.8f;
				system->col[2] = 1.0f;
				system->col[3] = trans_factor;
				}
			system->dcol[0]=0.0f;
			system->dcol[1]=0.0f;
			system->dcol[2]=0.0f;
			if( life > 0.0f )
				system->dcol[3] = -trans_factor / life;
			else
				system->dcol[3] = -0.1f;
			system->size = psize;
			system->use_wind = false;
			system->dsize = 0.0f;
			system->light_emitter = false;

			system->common_pos = O>>O;
			system->common_V = O>>O;

			for( int i = 0 ; i < ( nb < 0 ? 1 : nb ) ; i++ ) {
				system->pos[i] = O>>pos;
				system->V[i] = speed * Dir;
				}

			system->cur_idx++;
			}
		if( system->cur_idx == system->nb_particles ) {		// We're done
			EnterCS();
				particle_systems.push_back( system );
			LeaveCS();
			return NULL;
			}
		return system;
	}

	void PARTICLE_ENGINE::emit_lava(POINTF pos,VECTOR Dir,int tex,int nb,float speed,float life)
	{
		if( !lp_CONFIG->particle )	return;		// If particles are OFF don't add particles
		POINTF O;
		O.x=O.y=O.z=0.0f;
		if(game_cam!=NULL && ((VECTOR)(game_cam->Pos-(O>>pos))).Sq()>=game_cam->zfar2)	return;

		EnterCS();

		while(nb_part+nb>size)			// Si besoin alloue de la mémoire
			more_memory();

		for(int i=0;i<nb;i++) {
			if( !free_index_size )	break;

			uint32	cur_part = free_idx[--free_index_size];
			idx_list[index_list_size++] = cur_part;
			part[cur_part].px=-1;
			part[cur_part].Pos=O>>pos;
			float speed_mul=((rand_from_table()%100)*0.01f+0.01f);
			part[cur_part].V=speed_mul*speed*Dir;
			part[cur_part].life=life;
			part[cur_part].mass=1.0f;
			part[cur_part].smoking=-1.0f;
			part[cur_part].gltex=tex;
			part[cur_part].col[0]=1.0f;
			part[cur_part].col[1]=0.5f;
			part[cur_part].col[2]=0.5f;
			part[cur_part].col[3]=1.0f;
			part[cur_part].dcol[0]=0.0f;
			part[cur_part].dcol[1]=0.0f;
			part[cur_part].dcol[2]=0.0f;
			part[cur_part].dcol[3]=-1.0f/life;
			part[cur_part].angle=0.0f;
			part[cur_part].v_rot=(rand_from_table()%200)*0.01f-0.1f;
			part[cur_part].size=10.0f*(1.0f-speed_mul*0.9f);
			part[cur_part].use_wind=false;
			part[cur_part].dsize=0.0f;
			part[cur_part].ddsize=0.0f;
			part[cur_part].light_emitter=false;
			part[cur_part].slow_down=false;
			nb_part++;
			}
		LeaveCS();
	}

	void PARTICLE_ENGINE::make_shockwave(POINTF pos,int tex,int nb,float speed)
	{
		if( !lp_CONFIG->particle )	return;		// If particles are OFF don't add particles
		POINTF O;
		O.x=O.y=O.z=0.0f;

		if( nb_part + nb > 20000 )	nb = 20000 - nb_part;

		EnterCS();

		while(nb_part+nb>size)			// Si besoin alloue de la mémoire
			more_memory();

		float pre=speed*0.01f;
		for(int i=0;i<nb;i++) {
			if( !free_index_size )	break;
			if( nb_part > 20000 )	break;

			uint32	cur_part = free_idx[--free_index_size];
			idx_list[index_list_size++] = cur_part;
			part[cur_part].px=-1;
			part[cur_part].Pos=O>>pos;
			part[cur_part].V.y=0.0f;
			part[cur_part].V.x=((rand_from_table()%2001)-1000);
			part[cur_part].V.z=((rand_from_table()%2001)-1000);
			part[cur_part].V.Unit();
			part[cur_part].V=(pow((float)(rand_from_table()%100),2.0f)*0.0050f*(((rand_from_table()%2)==0) ? -1.0f : 1.0f)+50.0f)*pre*part[cur_part].V;
			if(tex==0)
				part[cur_part].life=3.0f+(rand_from_table()%200)*0.01f;
			else
				part[cur_part].life=3.0f;
			part[cur_part].mass=0.0f;
			part[cur_part].smoking = -1.0f;
			part[cur_part].gltex = 0;
			part[cur_part].col[0]=8.0f;
			part[cur_part].col[1]=8.0f;
			part[cur_part].col[2]=8.0f;
			part[cur_part].col[3]=1.2f;
			part[cur_part].dcol[0]=-0.3f/part[cur_part].life;
			part[cur_part].dcol[1]=-0.3f/part[cur_part].life;
			part[cur_part].dcol[2]=-0.3f/part[cur_part].life;
			part[cur_part].dcol[3]=-1.2f/part[cur_part].life;
			part[cur_part].angle=0.0f;
			part[cur_part].v_rot=(rand_from_table()%200)*0.01f-0.1f;
			part[cur_part].size=1.0f;
			part[cur_part].use_wind=false;
			if( tex == 1 ) {
				part[cur_part].dsize = 0.0f;
				part[cur_part].ddsize = 20.0f;
				}
			else {
				part[cur_part].dsize = 10.0f;
				part[cur_part].ddsize = 0.0f;
				}
			part[cur_part].light_emitter = false;
			part[cur_part].slow_down = true;
			part[cur_part].slow_factor = 0.01f;
			nb_part++;
			}

		LeaveCS();
	}

	void PARTICLE_ENGINE::make_nuke(POINTF pos,int tex,int nb,float speed)
	{
		if( !lp_CONFIG->particle )	return;		// If particles are OFF don't add particles
		POINTF O;
		O.x=O.y=O.z=0.0f;

		if( nb_part + nb > 20000 )	nb = 20000 - nb_part;

		EnterCS();

		while(nb_part+nb>size)			// Si besoin alloue de la mémoire
			more_memory();

		float pre=speed*0.01f;
		for(int i=0;i<nb;i++) {
			if( !free_index_size )	break;
			if( nb_part > 20000 )	break;

			uint32	cur_part = free_idx[--free_index_size];
			idx_list[index_list_size++] = cur_part;
			part[cur_part].px=-1;
			part[cur_part].Pos=O>>pos;
			part[cur_part].V.y=(rand_from_table()%9001)+1000;
			part[cur_part].V.x=((rand_from_table()%2001)-1000);
			part[cur_part].V.z=((rand_from_table()%2001)-1000);
			part[cur_part].V.Unit();
			part[cur_part].V=(100.0f - pow((float)(rand_from_table()%100),2.0f)*0.01f)*pre*part[cur_part].V;
			part[cur_part].life=3.0f + part[cur_part].V.Sq() * 0.0001f;
			part[cur_part].mass=1.0f;
			part[cur_part].smoking=-1.0f;
			part[cur_part].gltex=tex;
			part[cur_part].col[0]=1.0f;
			part[cur_part].col[1]=1.0f;
			part[cur_part].col[2]=1.0f;
			part[cur_part].col[3]=1.0f;
			part[cur_part].dcol[0]=0.0f;
			part[cur_part].dcol[1]=-0.8f/part[cur_part].life;
			part[cur_part].dcol[2]=0.0f;
			part[cur_part].dcol[3]=-1.0f/part[cur_part].life;
			part[cur_part].angle=0.0f;
			part[cur_part].v_rot = ((rand_from_table()%200)*0.01f-0.1f) * part[cur_part].V.Norm() * 0.015f / pre;
			part[cur_part].size=4.0f;
			part[cur_part].use_wind=true;
			part[cur_part].dsize=10.0f;
			part[cur_part].ddsize=0.0f;
			part[cur_part].light_emitter = (i & 1);
			part[cur_part].slow_down = true;
			part[cur_part].slow_factor = 1.0f;
			nb_part++;
			}
		LeaveCS();
	}

	void PARTICLE_ENGINE::make_smoke(POINTF pos,int tex,int nb,float speed,float mass,float ddsize,float alpha)
	{
		if( !lp_CONFIG->particle )	return;		// If particles are OFF don't add particles
		POINTF O;
		O.x=O.y=O.z=0.0f;
		if(game_cam!=NULL && ((VECTOR)(game_cam->Pos-(O>>pos))).Sq()>=game_cam->zfar2)	return;

		EnterCS();

		while(nb_part+nb>size)			// Si besoin alloue de la mémoire
			more_memory();

		float pre=speed*0.01f;
		for(int i=0;i<nb;i++) {
			if( !free_index_size )	break;

			uint32	cur_part = free_idx[--free_index_size];
			idx_list[index_list_size++] = cur_part;
			part[cur_part].px=-1;
			part[cur_part].Pos=O>>pos;
			part[cur_part].V.y=((rand_from_table()%1000)+1)*0.001f;
			part[cur_part].V.x=((rand_from_table()%2001)-1000)*0.001f;
			part[cur_part].V.z=((rand_from_table()%2001)-1000)*0.001f;
			part[cur_part].V.Unit();
			part[cur_part].V=((rand_from_table()%100)+1)*pre*part[cur_part].V;
			part[cur_part].life=5.0f;
			part[cur_part].mass=mass;
			part[cur_part].smoking=-1.0f;
			part[cur_part].gltex=tex;
			part[cur_part].col[0]=1.0f;
			part[cur_part].col[1]=1.0f;
			part[cur_part].col[2]=1.0f;
			part[cur_part].col[3]=alpha;
			part[cur_part].dcol[0]=0.0f;
			part[cur_part].dcol[1]=0.0f;
			part[cur_part].dcol[2]=0.0f;
			part[cur_part].dcol[3]=-0.2f*alpha;
			part[cur_part].angle=0.0f;
			part[cur_part].v_rot=(rand_from_table()%200)*0.01f-0.1f;
			part[cur_part].size=1.0f;
			part[cur_part].use_wind=mass!=0.0f ? true : false;
			part[cur_part].dsize=10.0f;
			part[cur_part].ddsize=ddsize;
			part[cur_part].light_emitter=false;
			part[cur_part].slow_down=false;
			nb_part++;
			}
		LeaveCS();
	}

	void PARTICLE_ENGINE::make_fire(POINTF pos,int tex,int nb,float speed)
	{
		if( !lp_CONFIG->particle )	return;		// If particles are OFF don't add particles
		POINTF O;
		O.x=O.y=O.z=0.0f;
		if(game_cam!=NULL && ((VECTOR)(game_cam->Pos-(O>>pos))).Sq()>=game_cam->zfar2)	return;

		EnterCS();

		while(nb_part+nb>size)			// Si besoin alloue de la mémoire
			more_memory();

		for(int i=0;i<nb;i++) {
			if( !free_index_size )	break;

			uint32	cur_part = free_idx[--free_index_size];
			idx_list[index_list_size++] = cur_part;
			part[cur_part].px=-1;
			part[cur_part].Pos=O>>pos;
			part[cur_part].V.y=((rand_from_table()%1000)+5000)*0.001f;
			part[cur_part].V.x=((rand_from_table()%2001)-1000)*0.001f;
			part[cur_part].V.z=((rand_from_table()%2001)-1000)*0.001f;
			part[cur_part].V.Unit();
			part[cur_part].V=((rand_from_table()%50)+51)*0.01f*speed*part[cur_part].V;
			part[cur_part].life=1.5f;
			part[cur_part].mass=-1.0f;
			part[cur_part].smoking=(rand_from_table()%60)*0.01f;
			part[cur_part].gltex=tex;
			part[cur_part].col[0]=1.0f;
			part[cur_part].col[1]=1.0f;
			part[cur_part].col[2]=1.0f;
			part[cur_part].col[3]=1.0f;
			part[cur_part].dcol[0]=-0.5f;
			part[cur_part].dcol[1]=-0.5f;
			part[cur_part].dcol[2]=-0.5f;
			part[cur_part].dcol[3]=-0.666667f;
			part[cur_part].angle=0.0f;
			part[cur_part].v_rot=(rand_from_table()%200)*0.01f-0.1f;
			part[cur_part].size=5.0f;
			part[cur_part].use_wind=true;
			part[cur_part].dsize=15.0f;
			part[cur_part].ddsize=-23.0f;
			part[cur_part].light_emitter=true;
			part[cur_part].slow_down=false;
			nb_part++;
			}
		LeaveCS();
	}

	void PARTICLE_ENGINE::move(float dt,VECTOR wind_dir,float g)
	{
		if( ( ( part == NULL || nb_part == 0 ) && particle_systems.empty() ) || dt == 0.0f ) {	rest( 1 );	return;	}

		EnterCS();

		VECTOR G;
		POINTF O;
		O.x=O.y=O.z=0.0f;
		G.x=G.y=G.z=0.0f;
		G.y=dt*g;
		wind_dir=dt*wind_dir;
		float factor=exp(-0.1f*dt);
		float factor2=exp(-dt);
		float dt_reduced = dt * 0.0025f;

		for( List< PARTICLE_SYSTEM* >::iterator i = particle_systems.begin() ; i != particle_systems.end() ; ) {
			(*i)->move( dt, &wind_dir, G.y, factor, factor2 );
			if( (*i)->life >= 0.0f )
				i++;
			else
				i = particle_systems.erase( i );
			LeaveCS();
			EnterCS();
			}

		uint32 i;

		for(sint32 e=0;e<index_list_size;e++) {
			if( !(e & 15) ) {
				LeaveCS();						// Pause to give the renderer the time to work and to go at the given engine speed (in ticks per sec.)
				EnterCS();
				}

			i = idx_list[e];
			part[i].life-=dt;
			if(part[i].life<0.0f) {
				free_idx[free_index_size++] = idx_list[e];
				idx_list[e--] = idx_list[--index_list_size];
				nb_part--;
				continue;
				}
			VECTOR RAND;
			RAND.x=((rand_from_table()&0x1FFF)-0xFFF)*dt_reduced;
			RAND.y=((rand_from_table()&0x1FFF)-0xFFF)*dt_reduced;
			RAND.z=((rand_from_table()&0x1FFF)-0xFFF)*dt_reduced;
			if(part[i].use_wind)
				part[i].V=part[i].V-part[i].mass*G+RAND+wind_dir;
			else
				part[i].V=part[i].V-part[i].mass*G+RAND;
			if(part[i].slow_down)
				part[i].V = exp( -dt * part[i].slow_factor ) * part[i].V;
			if(part[i].mass>0.0f)
				part[i].V=factor*part[i].V;
			else
				part[i].V=factor2*part[i].V;
			part[i].Pos=part[i].Pos+dt*part[i].V;
			part[i].size+=dt*part[i].dsize;
			part[i].dsize+=dt*part[i].ddsize;
			part[i].angle+=dt*part[i].v_rot;
			part[i].col[0]+=dt*part[i].dcol[0];
			part[i].col[1]+=dt*part[i].dcol[1];
			part[i].col[2]+=dt*part[i].dcol[2];
			part[i].col[3]+=dt*part[i].dcol[3];
			if(part[i].smoking>0.0f && part[i].life<part[i].smoking) {
				part[i].life = 1.0f;
				part[i].mass = -1.0f;
				part[i].col[0] = 0.2f;
				part[i].col[1] = 0.2f;
				part[i].col[2] = 0.2f;
				part[i].col[3] = 1.0f;
				part[i].gltex = 0;
				part[i].dcol[3] = -1.0f;
				part[i].size = 1.0f;
				part[i].dsize = 25.0f;
				part[i].smoking = -1.0f;
				part[i].light_emitter = false;
				}
			}
		LeaveCS();
	}

	void PARTICLE_ENGINE::draw(CAMERA *cam,int map_w,int map_h,int bloc_w,int bloc_h,byte **bmap)
	{
		if( ( part == NULL || nb_part == 0 ) && particle_systems.empty() )	return;		// no need to run the code if there is nothing to draw

		EnterCS();

		cam->SetView();

		glEnable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);
		glDepthMask(GL_FALSE);
//		glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);

		glDisableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);		// Les sommets
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glBindTexture(GL_TEXTURE_2D,parttex);

		glVertexPointer( 3, GL_FLOAT, 0, point);
		glTexCoordPointer(2, GL_FLOAT, 0, texcoord);
		glColorPointer(4,GL_UNSIGNED_BYTE,0,color);

		for( sint8 light_emitters = 0 ; light_emitters <= 1 ; light_emitters++ ) {

			uint32 i;
			sint32 j=-1;
			
			VECTOR A;
			VECTOR B;
			float oangle = 0.0f;
			int h_map_w=map_w>>1;
			int h_map_h=map_h>>1;
			for(sint32 e=0;e<index_list_size;e++) {					// Calcule la position des points
				i = idx_list[e];

				if(part[i].light_emitter != light_emitters )	continue;			// Two passes, one for normal particles, the second for particles that emits light

				if(part[i].px==-1) {
					part[i].px=((int)(part[i].Pos.x)+h_map_w)>>4;
					part[i].py=((int)(part[i].Pos.z)+h_map_h)>>4;
					}
				if(part[i].px>=0 && part[i].px<bloc_w && part[i].py>=0 && part[i].py<bloc_h) {
					if(!bmap[part[i].py][part[i].px])	continue;
					}
				else
					continue;	// Particule en dehors de la carte donc hors champ
				j++;
				if(j==0 || oangle!=part[i].angle) {
					oangle=part[i].angle;
					float cosinus=cos(part[i].angle);
					float sinus=sin(part[i].angle);
					A=(cosinus-sinus)*cam->Side+(sinus+cosinus)*cam->Up;
					B=(cosinus+sinus)*cam->Side+(sinus-cosinus)*cam->Up;
					if(cam->mirror) {
						A.y=-A.y;
						B.y=-B.y;
						}
					}
				int i_bis=j<<2;
				int i_qat=j<<4;
				point[i_bis++]=part[i].Pos-part[i].size*B;
				point[i_bis++]=part[i].Pos+part[i].size*A;
				point[i_bis++]=part[i].Pos+part[i].size*B;
				point[i_bis]=part[i].Pos-part[i].size*A;

				int i_ter=j<<3;
				float px=0.25f*(part[i].gltex&3)+0.001f;
				float py=0.25f*(part[i].gltex>>2)+0.001f;
				texcoord[i_ter++]=px;			texcoord[i_ter++]=py;
				texcoord[i_ter++]=px+0.248f;	texcoord[i_ter++]=py;
				texcoord[i_ter++]=px+0.248f;	texcoord[i_ter++]=py+0.248f;
				texcoord[i_ter++]=px;			texcoord[i_ter]=py+0.248f;

				uint32 col = 0;
				if( part[i].col[0] >= 0.0f && part[i].col[0] <= 1.0f )	col |= ((uint32)(part[i].col[0]*255));
				else col |= part[i].col[0] < 0.0f ? 0 : 0xFF;

				if( part[i].col[1] >= 0.0f && part[i].col[1] <= 1.0f )	col |= ((uint32)(part[i].col[1]*255))<<8;
				else col |= part[i].col[1] < 0.0f ? 0 : 0xFF00;

				if( part[i].col[2] >= 0.0f && part[i].col[2] <= 1.0f )	col |= ((uint32)(part[i].col[2]*255))<<16;
				else col |= part[i].col[2] < 0.0f ? 0 : 0xFF0000;

				if( part[i].col[3] >= 0.0f && part[i].col[3] <= 1.0f )	col |= ((uint32)(part[i].col[3]*255))<<24;
				else col |= part[i].col[3] < 0.0f ? 0 : 0xFF000000;

				((uint32*)color)[i_bis-3] = ((uint32*)color)[i_bis-2] = ((uint32*)color)[i_bis-1] = ((uint32*)color)[i_bis] = col;

				if( j >= 999 ) {
					glDrawElements(GL_QUADS, (j+1<<2),GL_UNSIGNED_SHORT,index);		// dessine le tout
					j = -1;
					}
				}

			if( j >= 0 )
				glDrawElements(GL_QUADS, (j+1<<2),GL_UNSIGNED_SHORT,index);		// dessine le tout

			glBlendFunc(GL_SRC_ALPHA,GL_ONE);
//			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			}

		LeaveCS();

		glDisableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);				// Vertices
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);

		float coeffs[] = {0.000000000001f, 0.0f, 1.0f / (SCREEN_H*SCREEN_H)};
		glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, coeffs);

		// Point size
		glPointParameterf (GL_POINT_SIZE_MAX, 3200000.0f);
		glPointParameterf (GL_POINT_SIZE_MIN, 1.0f);

		 // Set the texture center on the point
		glTexEnvf (GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

		// We're using point sprites
		glEnable (GL_POINT_SPRITE); 

		EnterCS();
		for( List< PARTICLE_SYSTEM* >::iterator i = particle_systems.begin() ; i != particle_systems.end() ; i++ )
			(*i)->draw();
		LeaveCS();
		glDisable (GL_POINT_SPRITE); 
		coeffs[0] = 1.0f;
		coeffs[1] = 0.0f;
		coeffs[2] = 0.0f;
		glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, coeffs);
		glPointSize(1.0f);

		glDisableClientState(GL_COLOR_ARRAY);
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);
	}

void PARTICLE_ENGINE::init(bool load)
{
	EnterCS();

	gltex.clear();

	particle_systems.clear();

	thread_running = false;
	thread_ask_to_stop = false;
	p_wind_dir = NULL;
	p_g = NULL;

	index_list_size=0;
	idx_list=NULL;
	free_index_size=0;
	free_idx=NULL;

	dsmoke=load;
	ntex=0;
	partbmp=NULL;
	if(load) {
		partbmp=create_bitmap_ex(32,256,256);
		BITMAP *bmp = load_bitmap("gfx/smoke.tga",NULL);//LoadMaskedTexBmp("gfx/smoke.tga","gfx/smokea.tga");
		gltex.push_back( gfx->make_texture( bmp ) );
		stretch_blit(bmp,partbmp,0,0,bmp->w,bmp->h,0,0,64,64);
		ntex=1;
		destroy_bitmap(bmp);
		allegro_gl_use_alpha_channel(true);
		allegro_gl_set_texture_format(GL_RGBA8);
		parttex=allegro_gl_make_texture(partbmp);
		allegro_gl_use_alpha_channel(false);
		glBindTexture(GL_TEXTURE_2D, parttex);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
		}
	size=0;
	nb_part=0;
	part=NULL;

	point=NULL;
	texcoord=NULL;
	index=NULL;
	color=NULL;

	LeaveCS();
}

void PARTICLE_ENGINE::destroy()
{
	EnterCS();

	for( int i = 0 ; i < gltex.size() ; i++ )
		gfx->destroy_texture( gltex[i] );
	gltex.clear();

	for( List< PARTICLE_SYSTEM* >::iterator i = particle_systems.begin() ; i != particle_systems.end() ; i++ )
		(*i)->destroy();

	particle_systems.clear();

	if(idx_list)	delete[]	idx_list;
	if(free_idx)	delete[]	free_idx;
	idx_list=NULL;	index_list_size=0;
	free_idx=NULL;	free_index_size=0;
	if(partbmp)
		destroy_bitmap(partbmp);
	partbmp=NULL;
	ntex=0;
	if(dsmoke)
		glDeleteTextures(1,&parttex);
	if(part)
		free(part);
	dsmoke=false;
	size=0;
	nb_part=0;
	part=NULL;

	if(point)
		free(point);
	if(texcoord)
		free(texcoord);
	if(index)
		free(index);
	if(color)
		free(color);
	point=NULL;
	texcoord=NULL;
	index=NULL;
	color=NULL;

	LeaveCS();
}

int PARTICLE_ENGINE::Run()
{
	thread_running = true;
	float dt = 1.0f / TICKS_PER_SEC;
	int particle_timer = msec_timer;
	int counter = 0;

	particle_engine_thread_sync = 0;

	while( !thread_ask_to_stop ) {
		counter++;

		move(dt,*p_wind_dir, *p_g);					// Animate particles

		ThreadSynchroniser->EnterSync();
		ThreadSynchroniser->LeaveSync();

		particle_engine_thread_sync = 1;

		while( particle_engine_thread_sync && !thread_ask_to_stop )	rest( 1 );			// Wait until other thread sync with this one
		}
	thread_running = false;
	thread_ask_to_stop = false;

	printf("particle engine: %f ticks/sec.\n", (float)(counter * 1000) / ( msec_timer - particle_timer ) );

	return 0;
}

void PARTICLE_ENGINE::SignalExitThread()
{
	if( thread_running )
		thread_ask_to_stop = true;
}
