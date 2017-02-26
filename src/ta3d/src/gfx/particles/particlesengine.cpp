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

#include <stdafx.h>
#include <misc/math.h>
#include <gfx/gl.extensions.h>
#include "particlesengine.h"
#include <misc/matrix.h>
#include <misc/timer.h>
#include <TA3D_NameSpace.h>
#include <ta3dbase.h>
#include <engine.h>
#include <EngineClass.h>


namespace TA3D
{


	PARTICLE_ENGINE	particle_engine;




	PARTICLE_ENGINE::PARTICLE_ENGINE()
		:nb_part(0), size(0), part(), parttex(0), partbmp(NULL), dsmoke(false),
		ntex(0), gltex(), point(NULL),
		texcoord(NULL), color(NULL), thread_running(false), thread_ask_to_stop(false),
		p_wind_dir(NULL), p_g(NULL), particle_systems()
	{
		init(false);
	}


	PARTICLE_ENGINE::~PARTICLE_ENGINE()
	{
		destroy();
	}



	int PARTICLE_ENGINE::addtex(const QString& file, const QString& filealpha)
	{
		MutexLocker locker(pMutex);

		dsmoke = true;
        if (partbmp.isNull())
			partbmp = gfx->create_surface_ex(32, 256, 256);
		QImage bmp;
        if (!filealpha.isEmpty())
			bmp = GFX::LoadMaskedTextureToBmp(file, filealpha); // Avec canal alpha séparé
		else
			bmp = gfx->load_image(file); // Avec canal alpha intégré ou Sans canal alpha

		gltex.push_back(gfx->make_texture(bmp));

		stretch_blit(bmp, partbmp, 0,0, bmp.width(), bmp.height(), 64 * (ntex & 3), 64 * (ntex >> 2), 64, 64);
		++ntex;
        gfx->destroy_texture(parttex);
		gfx->set_texture_format(gfx->defaultTextureFormat_RGBA());
		parttex = gfx->make_texture(partbmp, FILTER_TRILINEAR);

		return (ntex-1);
	}


	void PARTICLE_ENGINE::emit_part(const Vector3D &pos, const Vector3D &Dir,int tex,int nb,float speed,float life,float psize,bool white,float trans_factor)
	{
		MutexLocker locker(pMutex);
		if (!lp_CONFIG->particle)	// If particles are OFF don't add particles
			return;
		if (Camera::inGame != NULL && (Camera::inGame->pos - pos).sq() >= Camera::inGame->zfar2)
			return;

		for (int i = 0; i < nb; ++i)
		{
			PARTICLE new_part;
			new_part.px = -1;
			new_part.py = 0;
			new_part.slow_factor = 0.0f;
			new_part.Pos = pos;
			new_part.V = speed*Dir;
			new_part.life = life;
			new_part.mass = 0.0f;
			new_part.smoking = -1.0f;
			new_part.gltex = tex;
			if (white)
			{
				new_part.col[0] = 1.0f;
				new_part.col[1] = 1.0f;
				new_part.col[2] = 1.0f;
				new_part.col[3] = trans_factor;
			}
			else
			{
				new_part.col[0] = 0.8f;
				new_part.col[1] = 0.8f;
				new_part.col[2] = 1.0f;
				new_part.col[3] = trans_factor;
			}
			new_part.dcol[0] = 0.0f;
			new_part.dcol[1] = 0.0f;
			new_part.dcol[2] = 0.0f;
			if (life > 0.0f)
				new_part.dcol[3] = -trans_factor / life;
			else
				new_part.dcol[3] = -0.1f;
			new_part.angle = 0.0f;
			new_part.v_rot = float(Math::RandomTable() % 200) * 0.01f - 0.1f;
			new_part.size = psize;
			new_part.use_wind = false;
			new_part.dsize = 0.0f;
			new_part.ddsize = 0.0f;
			new_part.light_emitter = false;
			new_part.slow_down = false;
			part.push_back(new_part);
			++nb_part;
		}
	}

	ParticlesSystem *PARTICLE_ENGINE::emit_part_fast( ParticlesSystem *system, const Vector3D &pos, const Vector3D &Dir, int tex, int nb, float speed, float life, float psize, bool white, float trans_factor )
	{
		if (!lp_CONFIG->particle) // If particles are OFF don't add particles
			return NULL;
		if (tex < 0 || tex >= (int)gltex.size()) // We don't have that texture !!
			return NULL;
		if (system) // Step by step
		{
			system->pos[ system->cur_idx ] = pos;
			system->V[ system->cur_idx ] = speed * Dir;
			system->cur_idx++;
		}
		else
		{
			if (Camera::inGame != NULL && (Camera::inGame->pos - pos).sq()>=Camera::inGame->zfar2)
				return NULL;

			system = new ParticlesSystem;
			system->create( abs( nb ), gltex[ tex ] );

			system->life = life;
			system->mass = 0.0f;
			if (white)
			{
				system->col[0] = 1.0f;
				system->col[1] = 1.0f;
				system->col[2] = 1.0f;
				system->col[3] = trans_factor;
			}
			else
			{
				system->col[0] = 0.8f;
				system->col[1] = 0.8f;
				system->col[2] = 1.0f;
				system->col[3] = trans_factor;
			}
			system->dcol[0]=0.0f;
			system->dcol[1]=0.0f;
			system->dcol[2]=0.0f;
			if (life > 0.0f )
				system->dcol[3] = -trans_factor / life;
			else
				system->dcol[3] = -0.1f;
			system->size = psize;
			system->use_wind = false;
			system->dsize = 0.0f;
			system->light_emitter = false;

			system->common_pos.reset();

			for( int i = 0 ; i < ( nb < 0 ? 1 : nb ) ; ++i)
			{
				system->pos[i] = pos;
				system->V[i] = speed * Dir;
			}
			system->cur_idx++;
		}
		if (system->cur_idx == system->nb_particles)
		{
			pMutex.lock();
			particle_systems.push_back(system);
			pMutex.unlock();
			return NULL;
		}
		return system;
	}

	void PARTICLE_ENGINE::emit_lava(const Vector3D &pos, const Vector3D &Dir,int tex,int nb,float speed,float life)
	{
		if (!lp_CONFIG->particle ) // If particles are OFF don't add particles
			return;
		if (Camera::inGame!=NULL && (Camera::inGame->pos - pos).sq() >= Camera::inGame->zfar2)
			return;

		pMutex.lock();

		for (int i = 0 ; i < nb ; ++i)
		{
			PARTICLE new_part;
			new_part.px = -1;
			new_part.py = 0;
			new_part.slow_factor = 0.0f;
			new_part.Pos=pos;
			float speed_mul = (float(Math::RandomTable() % 100) * 0.01f + 0.01f);
			new_part.V = speed_mul * speed * Dir;
			new_part.life = life;
			new_part.mass = 1.0f;
			new_part.smoking = -1.0f;
			new_part.gltex = tex;
			new_part.col[0] = 1.0f;
			new_part.col[1] = 0.5f;
			new_part.col[2] = 0.5f;
			new_part.col[3] = 1.0f;
			new_part.dcol[0] = 0.0f;
			new_part.dcol[1] = 0.0f;
			new_part.dcol[2] = 0.0f;
			new_part.dcol[3] = -1.0f / life;
			new_part.angle = 0.0f;
			new_part.v_rot = float(Math::RandomTable() % 200) * 0.01f - 0.1f;
			new_part.size = 10.0f * (1.0f - speed_mul * 0.9f);
			new_part.use_wind = false;
			new_part.dsize = 0.0f;
			new_part.ddsize = 0.0f;
			new_part.light_emitter = false;
			new_part.slow_down = false;
			part.push_back(new_part);
			++nb_part;
		}
		pMutex.unlock();
	}

	void PARTICLE_ENGINE::make_shockwave(const Vector3D &pos,int tex,int nb,float speed)
	{
		if (!lp_CONFIG->particle) // If particles are OFF don't add particles
			return;
		if (nb_part + nb > 20000)
			nb = 20000 - nb_part;

		pMutex.lock();

		float pre = speed * 0.01f;
		for (int i = 0 ; i < nb ; ++i)
		{
			if (nb_part > 20000)
				break;

			PARTICLE new_part;
			new_part.px = -1;
			new_part.Pos = pos;
			new_part.V.y = 0.0f;
			new_part.V.x = float(((sint32)(Math::RandomTable() % 2001)) - 1000);
			new_part.V.z = float(((sint32)(Math::RandomTable() % 2001)) - 1000);
			new_part.V.unit();
			new_part.V = (powf(float(Math::RandomTable() % 100), 2.0f) * 0.0050f * (((Math::RandomTable() % 2) == 0) ? -1.0f : 1.0f) + 50.0f) * pre * new_part.V;
			if (tex == 0)
				new_part.life = 3.0f + float(Math::RandomTable() % 200) * 0.01f;
			else
				new_part.life = 3.0f;
			new_part.mass = 0.0f;
			new_part.smoking = -1.0f;
			new_part.gltex = 0;
			new_part.col[0] = 8.0f;
			new_part.col[1] = 8.0f;
			new_part.col[2] = 8.0f;
			new_part.col[3] = 1.2f;
			new_part.dcol[0] = -0.3f / new_part.life;
			new_part.dcol[1] = -0.3f / new_part.life;
			new_part.dcol[2] = -0.3f / new_part.life;
			new_part.dcol[3] = -1.2f / new_part.life;
			new_part.angle = 0.0f;
			new_part.v_rot = float(Math::RandomTable() % 200) * 0.01f - 0.1f;
			new_part.size = 1.0f;
			new_part.use_wind = false;
			if (tex == 1)
			{
				new_part.dsize = 0.0f;
				new_part.ddsize = 20.0f;
			}
			else
			{
				new_part.dsize = 10.0f;
				new_part.ddsize = 0.0f;
			}
			new_part.light_emitter = false;
			new_part.slow_down = true;
			new_part.slow_factor = 0.01f;
			part.push_back( new_part );
			++nb_part;
		}
		pMutex.unlock();
	}

	void PARTICLE_ENGINE::make_nuke(const Vector3D &pos,int tex,int nb,float speed)
	{
		if (!lp_CONFIG->particle) // If particles are OFF don't add particles
			return;
		if (nb_part + nb > 20000)
			nb = 20000 - nb_part;

		pMutex.lock();

		float pre = speed * 0.01f;
		for (int i=0;i<nb; ++i)
		{
			if (nb_part > 20000)
				break;

			PARTICLE new_part;

			new_part.px = -1;
			new_part.Pos = pos;
			new_part.V.y = float(Math::RandomTable() % 9001) + 1000.0f;
			new_part.V.x = float(((sint32)(Math::RandomTable() % 2001)) - 1000);
			new_part.V.z = float(((sint32)(Math::RandomTable() % 2001)) - 1000);
			new_part.V.unit();
			new_part.V = (100.0f - powf(float(Math::RandomTable() % 100), 2.0f) * 0.01f) * pre * new_part.V;
			new_part.life = 3.0f + new_part.V.sq() * 0.0001f;
			new_part.mass = 1.0f;
			new_part.smoking = -1.0f;
			new_part.gltex = tex;
			new_part.col[0] = 1.0f;
			new_part.col[1] = 1.0f;
			new_part.col[2] = 1.0f;
			new_part.col[3] = 1.0f;
			new_part.dcol[0] = 0.0f;
			new_part.dcol[1] = -0.8f / new_part.life;
			new_part.dcol[2] = 0.0f;
			new_part.dcol[3] = -1.0f / new_part.life;
			new_part.angle = 0.0f;
			new_part.v_rot = (float(Math::RandomTable() % 200) * 0.01f - 0.1f) * new_part.V.norm() * 0.015f / pre;
			new_part.size = 4.0f;
			new_part.use_wind = true;
			new_part.dsize = 10.0f;
			new_part.ddsize = 0.0f;
			new_part.light_emitter = (i & 1);
			new_part.slow_down = true;
			new_part.slow_factor = 1.0f;
			part.push_back( new_part );
			++nb_part;
		}
		pMutex.unlock();
	}

	void PARTICLE_ENGINE::make_smoke(const Vector3D &pos,int tex,int nb,float speed,float mass,float ddsize,float alpha)
	{
		if (!lp_CONFIG->particle ) // If particles are OFF don't add particles
			return;
		if (Camera::inGame!=NULL && (Camera::inGame->pos - pos).sq()>=Camera::inGame->zfar2)
			return;

		pMutex.lock();

		float pre=speed*0.01f;
		for (int i = 0; i < nb; ++i)
		{
			PARTICLE new_part;

			new_part.px = -1;
			new_part.Pos = pos;
			new_part.V.y = float((Math::RandomTable() % 1000) + 1) * 0.001f;
			new_part.V.x = float(((sint32)(Math::RandomTable() % 2001)) - 1000) * 0.001f;
			new_part.V.z = float(((sint32)(Math::RandomTable() % 2001)) - 1000) * 0.001f;
			new_part.V.unit();
			new_part.V = float((Math::RandomTable() % 100) + 1) * pre * new_part.V;

			new_part.life          = 3.0f;
			new_part.mass          = mass;
			new_part.smoking       = -1.0f;
			new_part.gltex         = tex;
			new_part.col[0]        = 1.0f;
			new_part.col[1]        = 1.0f;
			new_part.col[2]        = 1.0f;
			new_part.col[3]        = alpha;
			new_part.dcol[0]       = 0.0f;
			new_part.dcol[1]       = 0.0f;
			new_part.dcol[2]       = 0.0f;
			new_part.dcol[3]       = -0.3333f * alpha;
			new_part.angle         = 0.0f;
			new_part.v_rot         = float(Math::RandomTable() % 200) * 0.01f - 0.1f;
			new_part.size          = 1.0f;
			new_part.use_wind      = ((!Math::Zero(mass)) ? true : false);
			new_part.dsize         = 10.0f;
			new_part.ddsize        = ddsize;
			new_part.light_emitter = false;
			new_part.slow_down     = false;
			part.push_back(new_part);
			++nb_part;
		}
		pMutex.unlock();
	}

	void PARTICLE_ENGINE::make_dark_smoke(const Vector3D &pos,int tex,int nb,float speed,float mass,float ddsize,float alpha)
	{
		if (!lp_CONFIG->particle) // If particles are OFF don't add particles
			return;
		if (Camera::inGame != NULL && (Camera::inGame->pos - pos).sq() >= Camera::inGame->zfar2)
			return;

		pMutex.lock();

		float pre = speed * 0.01f;
		for(int i = 0 ; i < nb ; ++i)
		{
			PARTICLE new_part;

			new_part.px = -1;
			new_part.Pos = pos;
			new_part.V.y = float((Math::RandomTable() % 1000) + 1) * 0.001f;
			new_part.V.x = float(((sint32)(Math::RandomTable() % 2001)) - 1000) * 0.001f;
			new_part.V.z = float(((sint32)(Math::RandomTable() % 2001)) - 1000) * 0.001f;
			new_part.V.unit();
			new_part.V = float((Math::RandomTable() % 100) + 1) * pre * new_part.V;
			new_part.life = 1.0f;
			new_part.mass = mass;
			new_part.smoking = -1.0f;
			new_part.gltex = tex;
			new_part.col[0] = 0.2f;
			new_part.col[1] = 0.2f;
			new_part.col[2] = 0.2f;
			new_part.col[3] = alpha;
			new_part.dcol[0] = 0.0f;
			new_part.dcol[1] = 0.0f;
			new_part.dcol[2] = 0.0f;
			new_part.dcol[3] = -alpha;
			new_part.angle = 0.0f;
			new_part.v_rot = float(Math::RandomTable() % 200) * 0.01f - 0.1f;
			new_part.size = 1.0f;
			new_part.use_wind = (!Math::Zero(mass) ? true : false);
			new_part.dsize = 3.0f;
			new_part.ddsize = ddsize;
			new_part.light_emitter = false;
			new_part.slow_down = false;
			part.push_back( new_part );
			++nb_part;
		}
		pMutex.unlock();
	}

	void PARTICLE_ENGINE::make_fire(const Vector3D &pos,int tex,int nb,float speed)
	{
		if (!lp_CONFIG->particle ) // If particles are OFF don't add particles
			return;
		if (Camera::inGame != NULL && (Camera::inGame->pos - pos).sq() >= Camera::inGame->zfar2)
			return;

		pMutex.lock();

		for (int i = 0; i < nb; ++i)
		{
			PARTICLE new_part;

			new_part.px = -1;
			new_part.Pos = pos;
			new_part.V.y = float((Math::RandomTable() % 1000) + 5000) * 0.001f;
			new_part.V.x = float(((sint32)(Math::RandomTable() % 2001)) - 1000) * 0.001f;
			new_part.V.z = float(((sint32)(Math::RandomTable() % 2001)) - 1000) * 0.001f;
			new_part.V.unit();
			new_part.V = float((Math::RandomTable() % 50) + 51) * 0.01f * speed * new_part.V;
			new_part.life = 1.5f;
			new_part.mass = -1.0f;
			new_part.smoking = float(Math::RandomTable() % 60) * 0.01f;
			new_part.gltex = tex;
			new_part.col[0] = 1.0f;
			new_part.col[1] = 1.0f;
			new_part.col[2] = 1.0f;
			new_part.col[3] = 1.0f;
			new_part.dcol[0] = -0.5f;
			new_part.dcol[1] = -0.5f;
			new_part.dcol[2] = -0.5f;
			new_part.dcol[3] = -0.666667f;
			new_part.angle = 0.0f;
			new_part.v_rot = float(Math::RandomTable() % 200) * 0.01f - 0.1f;
			new_part.size = 5.0f;
			new_part.use_wind = true;
			new_part.dsize = 15.0f;
			new_part.ddsize = -23.0f;
			new_part.light_emitter = true;
			new_part.slow_down = false;
			part.push_back( new_part );
			++nb_part;
		}
		pMutex.unlock();
	}

	void PARTICLE_ENGINE::move(float dt, const Vector3D &wind_dir, float g)
	{
		pMutex.lock();
		if (((part.empty() || nb_part == 0) && particle_systems.empty()) || Math::Zero(dt))
		{
			pMutex.unlock();
			return;
		}

		const Vector3D G(0.0f, dt * g, 0.0f);
		const float factor = expf(-0.1f * dt);
		const float factor2 = expf(-dt);
		const float dt_reduced = dt * 0.0025f;

		for (std::vector< ParticlesSystem* >::iterator i = particle_systems.begin() ; i != particle_systems.end() ; )
		{
			(*i)->move( dt, factor, factor2 );
			if ((*i)->life >= 0.0f )
				++i;
			else
			{
				delete *i;
				const bool quit = (i + 1 == particle_systems.end());
				*i = particle_systems.back();
				particle_systems.pop_back();
				if (quit)
					break;
			}
		}

		pMutex.unlock();
		pMutex.lock();

		const Vector3D w_dir = dt * wind_dir;

		Vector3D RAND;
		for (std::vector<PARTICLE>::iterator e = part.begin() ; e != part.end() ;)
		{
			e->life -= dt;
			if (e->life < 0.0f)
			{
				bool quit =  (e + 1 == part.end());
				*e = part.back();
				part.pop_back();
				--nb_part;
				if (quit)
					break;
				continue;
			}
			RAND.x = float(((sint32)(Math::RandomTable() & 0x1FFF)) - 0xFFF) * dt_reduced;
			RAND.y = float(((sint32)(Math::RandomTable() & 0x1FFF)) - 0xFFF) * dt_reduced;
			RAND.z = float(((sint32)(Math::RandomTable() & 0x1FFF)) - 0xFFF) * dt_reduced;
			if (e->use_wind)
				e->V = e->V - e->mass * G + RAND + w_dir;
			else
				e->V = e->V - e->mass * G + RAND;
			if (e->slow_down)
				e->V = expf( -dt * e->slow_factor ) * e->V;
			if (e->mass>0.0f)
				e->V = factor * e->V;
			else
				e->V = factor2 * e->V;
			e->Pos = e->Pos + dt * e->V;
			e->size += dt * e->dsize;
			e->dsize += dt * e->ddsize;
			e->angle += dt * e->v_rot;
			e->col[0] += dt * e->dcol[0];
			e->col[1] += dt * e->dcol[1];
			e->col[2] += dt * e->dcol[2];
			e->col[3] += dt * e->dcol[3];
			if (e->smoking > 0.0f && e->life < e->smoking)
			{
				e->life = 1.0f;
				e->mass = -1.0f;
				e->col[0] = 0.2f;
				e->col[1] = 0.2f;
				e->col[2] = 0.2f;
				e->col[3] = 1.0f;
				e->gltex = 0;
				e->dcol[3] = -1.0f;
				e->size = 1.0f;
				e->dsize = 25.0f;
				e->smoking = -1.0f;
				e->light_emitter = false;
			}
			++e;
		}
		pMutex.unlock();
	}


	void PARTICLE_ENGINE::draw(Camera *cam)
	{
		if ((part.empty() || nb_part == 0) && particle_systems.empty())	// no need to run the code if there is nothing to draw
			return;

		pMutex.lock();

		if (!point)
			point = new Vector3D[4096];
		if (!texcoord)
			texcoord = new GLfloat[8192];
		if (!color)
			color = new GLubyte[16384];

		cam->setView(true);

		gfx->ReInitAllTex(true);

		glEnable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);
		glDepthMask(GL_FALSE);
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

		for (int light_emitters = 0; light_emitters <= 1; ++light_emitters)
		{
			sint32 j = -1;

			Vector3D A;
			Vector3D B;
			float oangle = 0.0f;
			const int h_map_w = the_map->map_w >> 1;
			const int h_map_h = the_map->map_h >> 1;
			for (std::vector<PARTICLE>::iterator e = part.begin(); e != part.end(); ++e) // Calcule la position des points
			{
				if (e->light_emitter != light_emitters) // Two passes, one for normal particles, the second for particles that emits light
					continue;

				if (e->px == -1)
				{
					e->px = short(((int)(e->Pos.x) + h_map_w) >> 4);
					e->py = short(((int)(e->Pos.z) + h_map_h) >> 4);
				}
				if (e->px >= 0 && e->px < the_map->bloc_w && e->py >= 0 && e->py < the_map->bloc_h)
				{
					if (!the_map->view(e->px, e->py))
						continue;
				}
				else
					continue;	// Particule en dehors de la carte donc hors champ
				++j;
				if (!j || !Math::Equals(oangle, e->angle))
				{
					oangle = e->angle;
					const float cosinus = cosf(e->angle);
					const float sinus = sinf(e->angle);
					A = (cosinus - sinus) * cam->side + (sinus + cosinus) * cam->up;
					B = (cosinus + sinus) * cam->side + (sinus - cosinus) * cam->up;
					if (cam->mirror)
					{
						A.y = -A.y;
						B.y = -B.y;
					}
				}
				int i_bis = j << 2;
				point[i_bis++] = e->Pos - e->size * B;
				point[i_bis++] = e->Pos + e->size * A;
				point[i_bis++] = e->Pos + e->size * B;
				point[i_bis] = e->Pos - e->size * A;

				int i_ter = j << 3;
				const float px = 0.25f * float(e->gltex & 3) + 0.001f;
				const float py = 0.25f * float(e->gltex >> 2) + 0.001f;
				texcoord[i_ter++] = px;			texcoord[i_ter++] = py;
				texcoord[i_ter++] = px+0.248f;	texcoord[i_ter++] = py;
				texcoord[i_ter++] = px+0.248f;	texcoord[i_ter++] = py+0.248f;
				texcoord[i_ter++] = px;			texcoord[i_ter]= py+0.248f;

				uint32 col = 0;
				if (e->col[0] >= 0.0f && e->col[0] <= 1.0f )
					col |= ((uint32)(e->col[0] * 255));
				else
					col |= e->col[0] < 0.0f ? 0 : 0xFF;

				if (e->col[1] >= 0.0f && e->col[1] <= 1.0f )
					col |= ((uint32)(e->col[1] * 255)) << 8;
				else
					col |= e->col[1] < 0.0f ? 0 : 0xFF00;

				if (e->col[2] >= 0.0f && e->col[2] <= 1.0f )
					col |= ((uint32)(e->col[2] * 255)) << 16;
				else
					col |= e->col[2] < 0.0f ? 0 : 0xFF0000;

				if (e->col[3] >= 0.0f && e->col[3] <= 1.0f )
					col |= ((uint32)(e->col[3] * 255)) << 24;
				else
					col |= e->col[3] < 0.0f ? 0 : 0xFF000000;

				((uint32*)color)[i_bis - 3] = ((uint32*)color)[i_bis - 2] = ((uint32*)color)[i_bis - 1] = ((uint32*)color)[i_bis] = col;

				if (j >= 1023 )
				{
					glDrawArrays( GL_QUADS, 0, (j + 1) << 2 );					// Draw everything
					j = -1;
				}
			}
			if (j >= 0 )
				glDrawArrays( GL_QUADS, 0, (j + 1) << 2 );					// Draw everything

			glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		}

		pMutex.unlock();

		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glDisableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);				// Vertices
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);

#warning TODO: implement point sprite
        float coeffs[] = {0.000000000001f, 0.0f, 1.0f / float(SCREEN_H * SCREEN_H)};
//		if (lp_CONFIG->ortho_camera)
//		{
//			coeffs[0] = Camera::inGame->zoomFactor * Camera::inGame->zoomFactor / 2.0f;
//			coeffs[1] = 0.0f;
//			coeffs[2] = 0.0f;
//            glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, coeffs);
//		}
//		else
//			glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, coeffs);

//		// Point size
//		glPointParameterf (GL_POINT_SIZE_MAX, 3200000.0f);
//		glPointParameterf (GL_POINT_SIZE_MIN, 1.0f);

//		// Set the texture center on the point
//        glTexEnvf (GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

//		// We're using point sprites
//        glEnable (GL_POINT_SPRITE);

		pMutex.lock();
		for (std::vector<ParticlesSystem*>::iterator i = particle_systems.begin() ; i != particle_systems.end() ; ++i)
			(*i)->draw();
		pMutex.unlock();
//		glDisable (GL_POINT_SPRITE);
//		coeffs[0] = 1.0f;
//		coeffs[1] = 0.0f;
//		coeffs[2] = 0.0f;
//		glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, coeffs);
//		glPointSize(1.0f);

        glDisableClientState(GL_COLOR_ARRAY);
        gfx->glDisable(GL_BLEND);
        gfx->glDepthMask(GL_TRUE);
        gfx->glEnable(GL_CULL_FACE);
	}

	void PARTICLE_ENGINE::drawUW()
	{
		if (particle_systems.empty())	// no need to run the code if there is nothing to draw
			return;

		Camera::inGame->setView(true);

		gfx->ReInitAllTex(true);

		glEnable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);

		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glDisableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);				// Vertices
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);

		float coeffs[] = {0.000000000001f, 0.0f, 1.0f / float(SCREEN_H * SCREEN_H)};
#warning TODO: implement point sprite
//		if (lp_CONFIG->ortho_camera)
//		{
//			coeffs[0] = Camera::inGame->zoomFactor * Camera::inGame->zoomFactor / 2.0f;
//			coeffs[1] = 0.0f;
//			coeffs[2] = 0.0f;
//			glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, coeffs);
//		}
//		else
//			glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, coeffs);

//		// Point size
//		glPointParameterf (GL_POINT_SIZE_MAX, 3200000.0f);
//		glPointParameterf (GL_POINT_SIZE_MIN, 1.0f);

//		// Set the texture center on the point
//		glTexEnvf (GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

//		// We're using point sprites
//		glEnable (GL_POINT_SPRITE);

		double eqn[4]= { 0.0f, -1.0f, 0.0f, the_map->sealvl };

        glClipPlane(GL_CLIP_PLANE1, eqn);
		glEnable(GL_CLIP_PLANE1);

		pMutex.lock();
		for (std::vector<ParticlesSystem*>::iterator i = particle_systems.begin() ; i != particle_systems.end() ; ++i)
			(*i)->draw();
		pMutex.unlock();

		glDisable(GL_CLIP_PLANE1);

//		glDisable (GL_POINT_SPRITE);
//		coeffs[0] = 1.0f;
//		coeffs[1] = 0.0f;
//		coeffs[2] = 0.0f;
//		glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, coeffs);
//		glPointSize(1.0f);

		glDisableClientState(GL_COLOR_ARRAY);
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);
	}

	void PARTICLE_ENGINE::init(bool load)
	{
		pMutex.lock();

		gltex.clear();
		particle_systems.clear();

		thread_running = false;
		thread_ask_to_stop = false;
		p_wind_dir = NULL;
		p_g = NULL;
		dsmoke=load;
        ntex = 0;
        partbmp = QImage();

		if (load)
		{
			partbmp = gfx->create_surface_ex(32,256,256);
			QImage bmp = gfx->load_image("gfx/smoke.tga");

			gltex.push_back(gfx->make_texture(bmp));
			stretch_blit(bmp, partbmp, 0, 0, bmp.width(), bmp.height(), 0, 0, 64, 64);
			ntex = 1;
			gfx->set_texture_format(gfx->defaultTextureFormat_RGBA());
			parttex = gfx->make_texture(partbmp, FILTER_TRILINEAR);
		}
		size=0;
		nb_part=0;
		part.clear();
		point=NULL;
		texcoord=NULL;
		color=NULL;

		pMutex.unlock();
	}

	void PARTICLE_ENGINE::destroy()
	{
		pMutex.lock();

		for (unsigned int i = 0 ; i < gltex.size() ; ++i)
			gfx->destroy_texture(gltex[i]);
		gltex.clear();

		for (std::vector<ParticlesSystem*>::iterator i = particle_systems.begin() ; i != particle_systems.end() ; ++i)
			(*i)->destroy();

		particle_systems.clear();

        partbmp = QImage();
		ntex = 0;
		gfx->destroy_texture(parttex);
		dsmoke = false;
		size = 0;
		nb_part = 0;
		part.clear();

		DELETE_ARRAY(point);
		DELETE_ARRAY(texcoord);
		DELETE_ARRAY(color);

		pMutex.unlock();
	}



	void PARTICLE_ENGINE::proc(void*)
	{
		thread_running = true;
		float dt = 1.0f / TICKS_PER_SEC;
		int particle_timer = msectimer();
		int counter = 0;

		while (!thread_ask_to_stop)
		{
			++counter;
			move(dt,*p_wind_dir, *p_g);	// Animate particles

			Engine::sync();
		}
		thread_running = false;
		thread_ask_to_stop = false;
		LOG_INFO("Particle engine: " << (float)(counter * 1000) / float(msectimer() - particle_timer) << " ticks/sec.");
	}


	void PARTICLE_ENGINE::signalExitThread()
	{
		if (thread_running)
			thread_ask_to_stop = true;
	}


} // namespace TA3D

