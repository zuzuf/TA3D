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

#include <yuni/yuni.h>
#include "weapons.ingame.h"
#include <EngineClass.h>
#include <misc/camera.h>
#include <gfx/gfx.h>
#include <engine.h>
#include <misc/bvh.h>
#include <UnitEngine.h>

namespace TA3D
{

	InGameWeapons weapons;


	void InGameWeapons::init(bool real)
	{
		pMutex.lock();

		thread_running = false;
		thread_ask_to_stop = false;

		idx_list.clear();
		free_idx.clear();
		nb_weapon = 0;
		weapon.clear();
		nuclogo.init();
		if (real)
		{
			File *file = VFS::Instance()->readFile("anims\\fx.gaf");
			if (file)
			{
				nuclogo.loadGAFFromRawData(file, Gaf::RawDataGetEntryIndex(file, "nuclogo"));
				nuclogo.convert();
				nuclogo.clean();
				delete file;
			}
		}
		pMutex.unlock();
	}


	void InGameWeapons::destroy()
	{
		destroyThread();

		pMutex.lock();
		idx_list.clear();
		free_idx.clear();
		weapon.clear();
		nuclogo.destroy();
		pMutex.unlock();
		init(false);
	}


	InGameWeapons::InGameWeapons()
	{
		init(false);
	}

	InGameWeapons::~InGameWeapons()
	{
		destroy();
	}


	int InGameWeapons::add_weapon(int weapon_id,int shooter)
	{
		if (weapon_id < 0)
			return -1;

		MutexLocker locker(pMutex);

		if (nb_weapon < weapon.size())// S'il y a encore de la place
		{
			uint32 i = free_idx.back();
			free_idx.pop_back();
			idx_list.push_back(i);
			++nb_weapon;
			weapon[i].init();
			weapon[i].weapon_id = weapon_id;
			weapon[i].shooter_idx = shooter;
			weapon[i].idx = i;
			weapon[i].f_time = weapon_manager.weapon[weapon_id].flighttime;
			return i;
		}
		weapon.resize( weapon.size() + 1 );

		uint32 index = uint32(weapon.size() - 1);
		idx_list.push_back( index );
		++nb_weapon;
		weapon.back().init();
		weapon.back().weapon_id = weapon_id;
		weapon.back().shooter_idx = shooter;
		weapon.back().idx = index;
		weapon.back().f_time = weapon_manager.weapon[weapon_id].flighttime;
		return index;
	}

	void InGameWeapons::move(float dt)
	{
		if (nb_weapon <= 0 || weapon.size() <= 0)
			return;

		std::vector<BVH_UnitTKit::T> allUnits;
		for (uint32 i = 0U ; i < units.max_unit ; ++i) // Compte les stocks de ressources et les productions
		{
			const Unit* const pUnit = &(units.unit[i]);
			if (!(pUnit->flags & 1))
				continue;
			const int type = pUnit->type_id;
			const UnitType* const pUnitType = (type >= 0) ? unit_manager.unit_type[type] : NULL;
			if (type < 0 || !pUnitType->model)
				continue;
			allUnits.push_back(std::make_pair(pUnit, std::make_pair(pUnit->Pos, pUnitType->model->size2)));
		}

		bvhUnits = new BVH<BVH_UnitTKit::T, BVH_UnitTKit>(allUnits.begin(), allUnits.end());

		pMutex.lock();
		if (nb_weapon <= 0 || weapon.size() <= 0)
		{
			pMutex.unlock();
			delete bvhUnits;
			return;
		}

		for (uint32 e = 0 ; e < idx_list.size() ; )
		{
			// TODO Check if it is really necessary by now
			pMutex.unlock();// Pause to give the renderer the time to work and to go at the given engine speed (in ticks per sec.)
			pMutex.lock();
			if (e >= idx_list.size())
				break;

			const uint32 i = idx_list[e];
			for ( ; weapon[i].ticks_to_compute > 0U && weapon[i].weapon_id >= 0 ; --weapon[i].ticks_to_compute)
				weapon[i].move(dt);
			weapon[i].move(dt);
			if (weapon[i].weapon_id < 0) // Remove it from the "alive" list
			{
				--nb_weapon;
				free_idx.push_back( i );
				idx_list[e] = idx_list.back();
				idx_list.pop_back();
			}
			else
				++e;
		}
		pMutex.unlock();
		delete bvhUnits;
	}



	void InGameWeapons::draw(bool underwater)
	{
		pMutex.lock();
		if(nb_weapon<=0 || weapon.size()<=0)
		{
			pMutex.unlock();
			return;
		}

		gfx->lock();

		for(std::vector<uint32>::iterator e = idx_list.begin() ; e != idx_list.end() ; ++e)
		{
			if(!underwater ^ (weapon[*e].Pos.y < the_map->sealvl))
				weapon[*e].draw();
		}

		gfx->unlock();
		pMutex.unlock();
	}



	void InGameWeapons::draw_mini(float map_w,float map_h,int mini_w,int mini_h)				// Repère les unités sur la mini-carte
	{
		MutexLocker locker(pMutex);

		if(nb_weapon <= 0 || weapon.size() <= 0)
			return;

		float rw = 128.0f * float(mini_w) / (252.0f * map_w);
		float rh = 128.0f * float(mini_h) / (252.0f * map_h);

		static std::vector<Vector2D> points;
		points.resize(idx_list.size());
		uint32 n(0);

		glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);
		glEnable(GL_TEXTURE_2D);
		for (std::vector<uint32>::iterator e = idx_list.begin() ; e != idx_list.end() ; ++e)
		{
			const uint32 i = *e;
			if(weapon_manager.weapon[weapon[i].weapon_id].cruise || weapon_manager.weapon[weapon[i].weapon_id].interceptor)
			{
				glEnable(GL_TEXTURE_2D);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
				const int idx = weapon[i].owner;
				GFX::PutTextureInsideRect(nuclogo.glbmp[idx], weapon[i].Pos.x * rw + 64.0f - float(nuclogo.ofs_x[idx]),
										  weapon[i].Pos.z * rh + 64.0f - float(nuclogo.ofs_y[idx]),
										  weapon[i].Pos.x * rw + 63.0f - float(nuclogo.ofs_x[idx] + nuclogo.w[idx]),
										  weapon[i].Pos.z * rh + 63.0f - float(nuclogo.ofs_y[idx] + nuclogo.h[idx]));
				glDisable(GL_BLEND);
			}
			else
			{
				points[n].x = weapon[i].Pos.x * rw + 64.0f;
				points[n].y = weapon[i].Pos.z * rh + 64.0f;
				++n;
			}
		}
		glDisable(GL_TEXTURE_2D);

		glDisableClientState(GL_NORMAL_ARRAY);              // Render all points in one pass
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer( 2, GL_FLOAT, 0, &(points.front()));

		glDrawArrays( GL_POINTS, 0, n );

		glEnable(GL_TEXTURE_2D);
	}


	void InGameWeapons::proc(void*)
	{
		thread_running = true;
		float dt = 1.0f / TICKS_PER_SEC;
		int weapon_timer = msec_timer;
		int counter = 0;

		while (!thread_ask_to_stop)
		{
			++counter;
			move(dt);					// Animate weapons
			features.move_forest(dt);			// Animate the forest

			Engine::sync();
		}
		thread_running = false;
		thread_ask_to_stop = false;
		LOG_INFO("Weapon engine: " << (float)(counter * 1000) / float(msec_timer - weapon_timer) << " ticks/sec");
	}


	void InGameWeapons::signalExitThread()
	{
		if (thread_running)
			thread_ask_to_stop = true;
	}



}
