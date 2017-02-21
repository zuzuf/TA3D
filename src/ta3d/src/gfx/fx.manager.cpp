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

#include "fx.manager.h"
#include <misc/math.h>
#include <mesh/instancing.h>         // Because we need the RenderQueue object
#include <ingame/players.h>
#include <EngineClass.h>


namespace TA3D
{

	FXManager fx_manager;
	Model* FXManager::currentParticleModel = NULL;



	int FXManager::add(const QString& filename, const QString& entryName, const Vector3D& pos, const float size)
	{
		if(Camera::inGame != NULL && (pos - Camera::inGame->pos).sq() >= Camera::inGame->zfar2)
			return -1;

		MutexLocker locker(pMutex);

		if (nb_fx + 1 > max_fx)
		{
			max_fx++;
			fx.resize( max_fx );
		}
		++nb_fx;
		int idx = -1;

		for (int i = 0; i < max_fx; ++i)
		{
			if (!fx[i].playing)
			{
				idx = i;
				break;
			}
		}

        const QString &tmp = "anims/" + filename + ".gaf";

        const QString &fullname = tmp + "-" + entryName;

		int anm_idx = findInCache(fullname);
		if (anm_idx == -1)
		{
			File *file;
            if (filename.toLower() != "fx")
				file = VFS::Instance()->readFile(tmp);
			else
				file = fx_data;
			if (file)
			{
				Gaf::Animation* anm = new Gaf::Animation();
				anm->loadGAFFromRawData(file, Gaf::RawDataGetEntryIndex(file, entryName));
				// Next line has been removed in order to remain thread safe, conversion is done in main thread
				//			anm->convert(false,true);
				pCacheIsDirty = true;				// Set cache as dirty so we will do conversion at draw time

				anm_idx = putInCache(fullname, anm);

				if(file != fx_data)
					delete file;
			}
		}
		else
			++use[anm_idx];
		fx[idx].load(anm_idx, pos, size);

		return idx;
	}


	void FXManager::loadData()
	{
		pMutex.lock();
		currentParticleModel = model_manager.get_model("fxpart");
		// Reload the texture for flashes
		if (!flash_tex.get())
			flash_tex.load("gfx/flash.tga");
		// Reload the texture for ripples
		if (!ripple_tex.get())
			ripple_tex.load("gfx/ripple.tga");
		// Reload textures for waves
		if (!wave_tex[0].get())
			wave_tex[0].load("gfx/wave0.tga");
		if (!wave_tex[1].get())
			wave_tex[1].load("gfx/wave1.tga");
		if (!wave_tex[2].get())
			wave_tex[2].load("gfx/wave2.tga");
		pMutex.unlock();
	}


	int FXManager::addFlash(const Vector3D& pos, const float size)
	{
		if(Camera::inGame != NULL && (pos - Camera::inGame->pos).sq() >= Camera::inGame->zfar2)
			return -1;

		MutexLocker locker(pMutex);
		if(nb_fx + 1 > max_fx)
		{
			max_fx++;
			fx.resize( max_fx );
		}
		++nb_fx;
		int idx=-1;
		for (int i=0;i<max_fx; ++i)
		{
			if(!fx[i].playing)
			{
				idx=i;
				break;
			}
		}
		fx[idx].load(-1, pos, size);

		return idx;
	}



	int FXManager::addWave(const Vector3D& pos,float size)
	{
		if (Camera::inGame != NULL && (pos-Camera::inGame->pos).sq() >= Camera::inGame->zfar2)
			return -1;

		MutexLocker locker(pMutex);

		if(nb_fx+1>max_fx)
		{
			max_fx++;
			fx.resize( max_fx );
		}
		++nb_fx;
		int idx = -1;
		for (int i = 0; i < max_fx; ++i)
		{
			if(!fx[i].playing)
			{
				idx=i;
				break;
			}
		}
		fx[idx].load(-2 - (Math::RandomTable() % 3), pos, size * 4.0f);

		return idx;
	}

	int FXManager::addRipple(const Vector3D& pos,float size)
	{
		if (Camera::inGame != NULL && (pos - Camera::inGame->pos).sq() >= Camera::inGame->zfar2)
			return -1;

		MutexLocker locker(pMutex);

		if(nb_fx + 1 > max_fx)
		{
			max_fx ++;
			fx.resize( max_fx );
		}
		++nb_fx;
		int idx = -1;
		for (int i = 0; i < max_fx; ++i)
		{
			if(!fx[i].playing)
			{
				idx = i;
				break;
			}
		}
		fx[idx].load(-5, pos, size);

		return idx;
	}




	void FXManager::doClearAllParticles()
	{
		pElectrics.clear();
		pParticles.clear();
	}

	void FXManager::init()
	{
		doClearAllParticles();
		currentParticleModel = NULL;
		pCacheIsDirty = false;
		fx_data = NULL;
		max_fx = 0;
		nb_fx = 0;
		fx.clear();

		hashName.clear();
		cacheName.clear();
		cacheAnm.clear();
		use.clear();

		flash_tex.destroy();
		wave_tex[0].destroy();
		wave_tex[1].destroy();
		wave_tex[2].destroy();
		ripple_tex.destroy();
	}



	void FXManager::destroy()
	{
		doClearAllParticles();

		flash_tex.destroy();
		ripple_tex.destroy();
		wave_tex[0].destroy();
		wave_tex[1].destroy();
		wave_tex[2].destroy();

		delete fx_data;
		fx.clear();
		hashName.clear();
		cacheName.clear();
		for (uint32 i = 0 ;i < cacheAnm.size() ; ++i)
			if (cacheAnm[i])
				delete cacheAnm[i];
		cacheAnm.clear();
		freeSlot.clear();
		use.clear();
		init();
	}

	void FXManager::draw(Camera& cam, float w_lvl, bool UW)
	{
		pMutex.lock();

		if (pCacheIsDirty)	// We have work to do
		{
			for (uint32 i = 0 ; i < cacheAnm.size() ; ++i)
			{
				if (cacheAnm[i])
					cacheAnm[i]->convert(false,true);
			}
			pCacheIsDirty = false;
		}

		glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

		glEnable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);
		glDepthMask(GL_FALSE);
		gfx->set_alpha_blending();
		cam.setView(true);
		if (UW)
		{
			for(int i = 0 ; i < max_fx ; ++i)
			{
				if (fx[i].playing && fx[i].Pos.y < w_lvl)
					fx[i].draw(cam, cacheAnm);
			}
		}
		else
		{
			for(int i = 0; i < max_fx; ++i)
			{
				if (fx[i].playing && fx[i].Pos.y >= w_lvl)
					fx[i].draw(cam, cacheAnm);
			}
		}
		gfx->unset_alpha_blending();
		glDepthMask(GL_TRUE);

		if (!UW && lp_CONFIG->explosion_particles && FXManager::currentParticleModel != NULL)
		{
			RenderQueue renderQueue(FXManager::currentParticleModel->id);
			for (ListOfParticles::iterator i = pParticles.begin(); i != pParticles.end(); ++i)
				(*i).draw( renderQueue );
			renderQueue.draw_queue();
		}

		glDisable(GL_TEXTURE_2D);
		if (!UW)
			for (ListOfElectrics::iterator i = pElectrics.begin(); i != pElectrics.end(); ++i)
				(*i).draw();

		pMutex.unlock();
	}

	void FXManager::drawWaterDistortions()
	{
		pMutex.lock();

		glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

		glEnable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		for(int i = 0; i < max_fx; ++i)
			if (fx[i].playing)
				fx[i].drawWaterDistortions();
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);

		pMutex.unlock();
	}

	void FXManager::addParticle(const Vector3D& p, const Vector3D& s, const float l)
	{
		if (lp_CONFIG->explosion_particles)
		{
			pMutex.lock();
			pParticles.push_back(FXParticle(p, s, l));
			pMutex.unlock();
		}
	}

	void FXManager::addExplosion(const Vector3D& p, const int n, const float power)
	{
		if (!lp_CONFIG->explosion_particles)
			return;

		if (the_map)            // Visibility test
		{
			const int px = ((int)(p.x+0.5f) + the_map->map_w_d)>>4;
			const int py = ((int)(p.z+0.5f) + the_map->map_h_d)>>4;
			if (px < 0 || py < 0 || px >= the_map->bloc_w || py >= the_map->bloc_h)	return;
			const byte player_mask = byte(1 << players.local_human_id);
			if (the_map->view(px, py) != 1
				|| !(the_map->sight_map(px, py ) & player_mask))	return;
		}

		pMutex.lock();
		const float rev = 5.0f / (the_map->ota_data.gravity + 0.1f);
		for (int i = 0 ; i < n ; ++i)
		{
			const float a = static_cast<float>(Math::RandomTable() % 36000) * 0.01f * DEG2RAD;
			const float b = static_cast<float>(Math::RandomTable() % 18000) * 0.01f * DEG2RAD;
			const float s = power * (static_cast<float>(Math::RandomTable() % 9001) * 0.0001f + 0.1f);
			const float scosb = s * cosf(b);
			const Vector3D vs(cosf(a) * scosb,
							  s * sinf(b),
							  sinf(a) * scosb);
			const float l = static_cast<float>(Math::RandomTable() % 1001) * 0.001f - 0.5f + Math::Min(rev * vs.y, 10.0f);

			pParticles.push_back(FXParticle(p, vs, l));
		}
		pMutex.unlock();
	}


	void FXManager::addExplosion(const Vector3D& p, const Vector3D& s, const int n, const float power)
	{
		if (!lp_CONFIG->explosion_particles)
			return;

		if (the_map)            // Visibility test
		{
			const int px = ((int)(p.x + 0.5f) + the_map->map_w_d) >> 4;
			const int py = ((int)(p.z + 0.5f) + the_map->map_h_d) >> 4;
			if (px < 0 || py < 0 || px >= the_map->bloc_w || py >= the_map->bloc_h)
				return;
			const byte player_mask = byte(1 << players.local_human_id);
			if (the_map->view(px, py) != 1 || !(the_map->sight_map(px, py) & player_mask))
				return;
		}

		// Temporary variables
		float a;
		float b;
		float speed;
		float l;
		Vector3D vs;

		// Locking...
		pMutex.lock();

		// Foreach particle
		for (int i = 0; i < n; ++i)
		{
			a     = static_cast<float>(Math::RandomTable() % 36000) * 0.01f * DEG2RAD;
			b     = static_cast<float>(Math::RandomTable() % 18000) * 0.01f * DEG2RAD;
			speed = power * (static_cast<float>(Math::RandomTable() % 9001) * 0.0001f + 0.1f);
			vs.x  = speed * cosf(a) * cosf(b);
			vs.y  = speed * sinf(b);
			vs.z  = speed * sinf(a) * cosf(b);
			l     = Math::Min(5.0f * vs.y / (the_map->ota_data.gravity + 0.1f), 10.0f);

			// Adding the new particle into the list
			pParticles.push_back(FXParticle(p, s + vs, l));
		}
		// Unlokcing
		pMutex.unlock();
	}


	void FXManager::addElectric(const Vector3D& p)
	{
		pMutex.lock();
		pElectrics.push_back(FXElectric(p));
		pMutex.unlock();
	}

	int FXManager::putInCache(const QString& filename, Gaf::Animation* anm)
	{
		// Already available in the cache ?
		const int is_in = findInCache(filename);
		if (is_in >= 0)
			return is_in;

		int idx = -1;
		if (freeSlot.empty())
		{
			idx = int(cacheAnm.size());
			cacheName.push_back(QString());
			cacheAnm.push_back(NULL);
			use.push_back(0);
		}
		else
		{
			idx = freeSlot.front();
			freeSlot.pop_front();
		}
		use[idx] = 1;
		cacheAnm[idx] = anm;
		cacheName[idx] = filename;
		hashName[filename] = idx;

		return idx;
	}


	int FXManager::findInCache(const QString& filename) const
	{
		HashMap<int>::Sparse::const_iterator it = hashName.find(filename);
		if (it == hashName.end())
			return -1;
		return *it;
	}


	void FXManager::doMoveAllParticles(const float& dt)
	{
		for (ListOfParticles::iterator i = pParticles.begin(); i != pParticles.end(); )
		{
			if ((*i).move(dt))
			{
				if (i + 1 == pParticles.end())
				{
					pParticles.pop_back();
					break;
				}
				*i = pParticles.back();
				pParticles.pop_back();
			}
			else
				++i;
		}
		for (ListOfElectrics::iterator i = pElectrics.begin(); i != pElectrics.end(); )
		{
			if ((*i).move(dt))
			{
				if (i + 1 == pElectrics.end())
				{
					pElectrics.pop_back();
					break;
				}
				*i = pElectrics.back();
				pElectrics.pop_back();
			}
			else
				++i;
		}
	}


	void FXManager::doMoveAllFX(const float& dt)
	{
		for (int i = 0; i < max_fx; ++i)
		{
			if(fx[i].move(dt, cacheAnm))
			{
				if (fx[i].anm == -1 || fx[i].anm == -2 || fx[i].anm == -3 || fx[i].anm == -4 || fx[i].anm == -5)
				{
					--nb_fx;
					continue;		// Flash, ripple or Wave
				}
				--use[fx[i].anm];
				--nb_fx;
				if (use[fx[i].anm] <= 0) // Animation used nowhere
				{
					hashName.erase(cacheName[fx[i].anm]);
					cacheName[fx[i].anm].clear();
					if (cacheAnm[fx[i].anm])
					{
						delete cacheAnm[fx[i].anm];
						freeSlot.push_back(fx[i].anm);
					}
					cacheAnm[fx[i].anm] = NULL;
				}
			}
		}

	}


	void FXManager::move(const float dt)
	{
		pMutex.lock();
		doMoveAllFX(dt);
		doMoveAllParticles(dt);
		pMutex.unlock();
	}


	FXManager::FXManager()
	{
		init();
	}

	FXManager::~FXManager()
	{
		destroy();
	}


} // namespace TA3D
