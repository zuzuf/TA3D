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
|                                        fx.cpp                                      |
|   This modules contains the special FX engine, it draws sprites, waves, light      |
| halos, bouncing particles, ...                                                     |
|                                                                                    |
\-----------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "matrix.h"
#include "TA3D_NameSpace.h"
#include "ta3dbase.h"
#include "EngineClass.h"
#include "tdf.h"
#include "fx.h"
#include "particles.h"
#include "3do.h"

FX_MANAGER	fx_manager;

MODEL       *particle_model = NULL;

void FX::draw(CAMERA *cam, MAP *map, ANIM **anims)
{
	if(!playing)	return;
	if(map!=NULL) {
		int px=(int)(Pos.x+map->map_w*0.5f)>>4;
		int py=(int)(Pos.z+map->map_h*0.5f)>>4;
		if(px<0 || py<0 || px>=map->bloc_w || py>=map->bloc_h)	return;
		byte player_mask = 1 << players.local_human_id;
		if((map->view[py][px]!=1
		|| !(map->sight_map->line[py][px]&player_mask))
		&& ( anm > -2 || anm < -4 ))	return;
		}

	if( anm == -1 ) {				// It's a flash
		glDisable( GL_DEPTH_TEST );
		glBindTexture(GL_TEXTURE_2D, fx_manager.flash_tex);
		glBlendFunc(GL_ONE,GL_ONE);

		float rsize = -4.0f * time * ( time - 1.0f ) * size;

		glBegin(GL_QUADS);
			glTexCoord2f(0.0f,0.0f);	glVertex3f(Pos.x - rsize,Pos.y,Pos.z - rsize);
			glTexCoord2f(1.0f,0.0f);	glVertex3f(Pos.x + rsize,Pos.y,Pos.z - rsize);
			glTexCoord2f(1.0f,1.0f);	glVertex3f(Pos.x + rsize,Pos.y,Pos.z + rsize);
			glTexCoord2f(0.0f,1.0f);	glVertex3f(Pos.x - rsize,Pos.y,Pos.z + rsize);
		glEnd();
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glEnable( GL_DEPTH_TEST );
		
		return;
		}
	if( anm == -2 || anm == -3 || anm == -4 ) {				// It's a wave
		glPolygonOffset(0.0f, 0.0f);
		glBindTexture(GL_TEXTURE_2D, fx_manager.wave_tex[ anm + 4 ]);
		gfx->set_alpha_blending();

		glPushMatrix();

		glTranslatef( Pos.x, Pos.y, Pos.z );
		glRotatef( size, 0.0f, 1.0f, 0.0f );

		float wsize = 24.0f;
		float hsize = 8.0f;
		float dec = time * 0.125f;

		glColor4f( 1.0f, 1.0f, 1.0f, 1.0f - 0.5f * fabs( 2.0f - time ) );

		glBegin(GL_QUADS);
			glTexCoord2f(0.0f,dec );		glVertex3f(-wsize,4.0f,-hsize);
			glTexCoord2f(1.0f,dec );		glVertex3f(wsize,4.0f,-hsize);
			glTexCoord2f(1.0f,dec+1.0f );	glVertex3f(wsize,0.0f,hsize);
			glTexCoord2f(0.0f,dec+1.0f );	glVertex3f(-wsize,0.0f,hsize);
		glEnd();

		glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

		glPopMatrix();

		gfx->unset_alpha_blending();
		
		glPolygonOffset(0.0f,-1600.0f);
		return;
		}
	if( anm == -5 ) {				// It's a ripple
		glPolygonOffset(0.0f,0.0f);
		glBindTexture(GL_TEXTURE_2D, fx_manager.ripple_tex);
		gfx->set_alpha_blending();

		glPushMatrix();

		glTranslatef( Pos.x, Pos.y, Pos.z );
		glRotatef( size*time, 0.0f, 1.0f, 0.0f );

		float rsize = 16.0f * time;

		glColor4f( 1.0f, 1.0f, 1.0f, 0.5f - 0.25f * time );

		glBegin(GL_QUADS);
			glTexCoord2f(0.0f,0.0f );		glVertex3f(-rsize,0.0f,-rsize);
			glTexCoord2f(1.0f,0.0f );		glVertex3f(rsize,0.0f,-rsize);
			glTexCoord2f(1.0f,1.0f );	glVertex3f(rsize,0.0f,rsize);
			glTexCoord2f(0.0f,1.0f );	glVertex3f(-rsize,0.0f,rsize);
		glEnd();

		glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

		glPopMatrix();

		gfx->unset_alpha_blending();
		
		glPolygonOffset(0.0f,-1600.0f);
		return;
		}

	if(anims[anm]==NULL) {
		playing=false;
		return;
		}
	int img=(int)(time*15.0f);
	float wsize=size*anims[anm]->w[img];
	float hsize=size*anims[anm]->h[img];
	glBindTexture(GL_TEXTURE_2D,anims[anm]->glbmp[img]);

	float hux=hsize*cam->Up.x;
	float wsx=wsize*cam->Side.x;
	float huy=hsize*cam->Up.y;
	float wsy=wsize*cam->Side.y;
	float huz=hsize*cam->Up.z;
	float wsz=wsize*cam->Side.z;

	glPushMatrix();
	glTranslatef( Pos.x, Pos.y, Pos.z );

	if(cam->mirror) {
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f,0.0f);	glVertex3f(  hux-wsx, -huy+wsy,  huz-wsz);
			glTexCoord2f(1.0f,0.0f);	glVertex3f(  hux+wsx, -huy-wsy,  huz+wsz);
			glTexCoord2f(1.0f,1.0f);	glVertex3f( -hux+wsx,  huy-wsy, -huz+wsz);
			glTexCoord2f(0.0f,1.0f);	glVertex3f( -hux-wsx,  huy+wsy, -huz-wsz);
		glEnd();
		}
	else {
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f,0.0f);	glVertex3f(  hux-wsx,  huy-wsy,  huz-wsz);
			glTexCoord2f(1.0f,0.0f);	glVertex3f(  hux+wsx,  huy+wsy,  huz+wsz);
			glTexCoord2f(1.0f,1.0f);	glVertex3f( -hux+wsx, -huy+wsy, -huz+wsz);
			glTexCoord2f(0.0f,1.0f);	glVertex3f( -hux-wsx, -huy-wsy, -huz-wsz);
		glEnd();
		}
	glPopMatrix();
}

int FX_MANAGER::add(char *filename,char *entry_name,VECTOR Pos,float size)
{
	if(game_cam!=NULL && ((VECTOR)(Pos-game_cam->Pos)).Sq()>=game_cam->zfar2)	return -1;

	EnterCS();

	if(nb_fx+1>max_fx) {
		max_fx+=100;
		FX *n_fx=(FX*) malloc(sizeof(FX)*max_fx);
		memcpy(n_fx,fx,sizeof(FX)*(max_fx-100));
		for(int i=max_fx-100;i<max_fx;i++)
			n_fx[i].init();
		free(fx);
		fx=n_fx;
		}
	nb_fx++;
	int idx=-1;
	for(int i=0;i<max_fx;i++)
		if(!fx[i].playing) {
			idx=i;
			break;
			}
	String tmp = String("anims\\") + String(filename) + String(".gaf");
	String fullname = tmp + String("-") + String(entry_name);
	int anm_idx = is_in_cache((char*)fullname.c_str());
	if(anm_idx==-1) {
		byte *data;
		if(strcasecmp(filename,"fx"))
			data = HPIManager->PullFromHPI(tmp);
		else
			data = fx_manager.fx_data;
		if(data) {
			ANIM *anm=new ANIM;
			anm->init();
			anm->load_gaf(data,get_gaf_entry_index(data,entry_name));
						// Next line has been removed in order to remain thread safe, conversion is done in main thread
//			anm->convert(false,true);
			cache_is_dirty = true;				// Set cache as dirty so we will do conversion at draw time

			anm_idx=put_in_cache((char*)fullname.c_str(),anm);

			if(data!=fx_manager.fx_data)
				free(data);
			}
		}
	else
		use[anm_idx]++;
	fx[idx].load(anm_idx,Pos,size);

	LeaveCS();

	return idx;
}

void FX_MANAGER::load_data()
{
    particle_model = model_manager.get_model("fxpart");
	if( flash_tex == 0 )
		flash_tex = gfx->load_texture( "gfx/flash.tga" );
	if( ripple_tex == 0 )
		ripple_tex = gfx->load_texture( "gfx/ripple.tga" );
	if( wave_tex[0] == 0 )
		wave_tex[0] = gfx->load_texture( "gfx/wave0.tga" );
	if( wave_tex[1] == 0 )
		wave_tex[1] = gfx->load_texture( "gfx/wave1.tga" );
	if( wave_tex[2] == 0 )
		wave_tex[2] = gfx->load_texture( "gfx/wave2.tga" );
}

int FX_MANAGER::add_flash(VECTOR Pos,float size)
{
	if(game_cam!=NULL && ((VECTOR)(Pos-game_cam->Pos)).Sq()>=game_cam->zfar2)	return -1;

	EnterCS();

	if(nb_fx+1>max_fx) {
		max_fx+=100;
		FX *n_fx=(FX*) malloc(sizeof(FX)*max_fx);
		memcpy(n_fx,fx,sizeof(FX)*(max_fx-100));
		for(int i=max_fx-100;i<max_fx;i++)
			n_fx[i].init();
		free(fx);
		fx=n_fx;
		}
	nb_fx++;
	int idx=-1;
	for(int i=0;i<max_fx;i++)
		if(!fx[i].playing) {
			idx=i;
			break;
			}
	fx[idx].load(-1,Pos,size);

	LeaveCS();

	return idx;
}

int FX_MANAGER::add_wave(VECTOR Pos,float size)
{
	if(game_cam!=NULL && ((VECTOR)(Pos-game_cam->Pos)).Sq()>=game_cam->zfar2)	return -1;

	EnterCS();

	if(nb_fx+1>max_fx) {
		max_fx+=100;
		FX *n_fx=(FX*) malloc(sizeof(FX)*max_fx);
		memcpy(n_fx,fx,sizeof(FX)*(max_fx-100));
		for(int i=max_fx-100;i<max_fx;i++)
			n_fx[i].init();
		free(fx);
		fx=n_fx;
		}
	nb_fx++;
	int idx=-1;
	for(int i=0;i<max_fx;i++)
		if(!fx[i].playing) {
			idx=i;
			break;
			}
	fx[idx].load(-2-(rand_from_table()%3),Pos,size*4.0f);

	LeaveCS();

	return idx;
}

int FX_MANAGER::add_ripple(VECTOR Pos,float size)
{
	if(game_cam!=NULL && ((VECTOR)(Pos-game_cam->Pos)).Sq()>=game_cam->zfar2)	return -1;

	EnterCS();

	if(nb_fx+1>max_fx) {
		max_fx+=100;
		FX *n_fx=(FX*) malloc(sizeof(FX)*max_fx);
		memcpy(n_fx,fx,sizeof(FX)*(max_fx-100));
		for(int i=max_fx-100;i<max_fx;i++)
			n_fx[i].init();
		free(fx);
		fx=n_fx;
		}
	nb_fx++;
	int idx=-1;
	for(int i=0;i<max_fx;i++)
		if(!fx[i].playing) {
			idx=i;
			break;
			}
	fx[idx].load(-5,Pos,size);

	LeaveCS();

	return idx;
}

void FX_MANAGER::init()
{
	particles.clear();

    particle_model = NULL;

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

void FX_MANAGER::destroy()
{
	particles.clear();

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

void FX_MANAGER::draw(CAMERA *cam, MAP *map, float w_lvl, bool UW)
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
			if( fx[i].playing && fx[i].Pos.y >= w_lvl )
				fx[i].draw(cam,map,cache_anm);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0.0f,0.0f);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

	if(!UW)
	    foreach( particles, i )
		    i->draw();

	LeaveCS();
}

void FX_MANAGER::add_particle( VECTOR &P, VECTOR &S, float L )
{
	EnterCS();
		particles.push_back( FX_PARTICLE( P, S, L ) );
	LeaveCS();
}

void FX_MANAGER::add_explosion( VECTOR &P, int n, float power )
{
    EnterCS();
        for( int i = 0 ; i < n ; i++ ) {
            VECTOR S;
            float a = (rand_from_table() % 36000) * 0.01f * DEG2RAD;
            float b = (rand_from_table() % 18000) * 0.01f * DEG2RAD;
            float s = power * ( (rand_from_table() % 9001) * 0.0001f + 0.1f );
            S.x = s * cos( a ) * cos( b );
            S.y = s * sin( b );
            S.z = s * sin( a ) * cos( b );

            float L = min( 5.0f * S.y / (the_map->ota_data.gravity + 0.1f), 10.0f );
            
            particles.push_back( FX_PARTICLE( P, S, L ) );
            }
    LeaveCS();
}

FX_PARTICLE::FX_PARTICLE( VECTOR &P, VECTOR &S, float L )
{
	Pos = P;
	Speed = S;
	life = L;
	timer = 0.0f;
}

bool FX_PARTICLE::move( float dt )
{
	life -= dt;
	timer += dt;
	
	Speed.y -= dt * the_map->ota_data.gravity;		// React to gravity
	Pos = Pos + dt * Speed;
	
	float min_h = the_map->get_unit_h( Pos.x, Pos.z );
	if( Pos.y < min_h ) {							// Bouncing on the map :)
		Pos.y = 2.0f * min_h - Pos.y;
		float dx = the_map->get_unit_h( Pos.x + 16.0f, Pos.z );
		float dz = the_map->get_unit_h( Pos.x, Pos.z + 16.0f );
		VECTOR Normal( dx, -16.0f, dz );
		Normal.Unit();
		
		if( Speed % Normal < 0.0f )
    		Speed = Speed - (1.5f * (Speed % Normal)) * Normal;
		}
	
	while( timer >= 0.1f ) {			// Emit smoke
		timer -= 0.1f;
		particle_engine.make_dark_smoke( Pos, 0, 1, 0.0f, -1.0f, -1.0f, 0.5f );
		}
		
	return life <= 0.0f;			// When it shoud die, return true
}

void FX_PARTICLE::draw()
{
    if( particle_model == NULL )    return;     // Nothing we can draw
	glPushMatrix();
	glTranslatef( Pos.x, Pos.y, Pos.z );
	glScalef( 0.2f, 0.2f, 0.2f );

	particle_model->draw( life );

	glPopMatrix();
}
