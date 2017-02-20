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

#include <misc/matrix.h>
#include <TA3D_NameSpace.h>
#include <ta3dbase.h>
#include <scripts/cob.h>             // To read and execute scripts
#include <EngineClass.h>
#include <UnitEngine.h>
#include <misc/paths.h>
#include <misc/math.h>
#include <logs/logs.h>
#include <ingame/players.h>
#include "ai.controller.h"
#include "ai.h"
#include <QFile>
#include <QDataStream>

namespace TA3D
{



	//#define AI_DEBUG

/*	static byte int2brain_value(int a)	// Convert an integer into a value a neural network can understand
	{
		if (a == 0)  return BRAIN_VALUE_NULL;
		if (a <= 5)  return BRAIN_VALUE_LOW;
		if (a <= 15) return BRAIN_VALUE_MEDIUM;
		if (a <= 50) return BRAIN_VALUE_HIGH;
		return BRAIN_VALUE_MAX;
	}


	static int get_bits(float bits[], byte v, int pos)	// Fill the array (neural network input)
	{
		for (int i = 0; i < BRAIN_VALUE_BITS; ++i)
			bits[pos++] = ((v >> i) & 1) ? 1.0f : 0.0f;
		return pos;
	}*/


	void AiController::scan_unit()							// Scan the units the AI player currently has
	{
		if (weights.empty())
		{
			weights.resize(unit_manager.nb_unit);

			for (int i = 0 ; i < unit_manager.nb_unit ; ++i)
			{
				if ( ToLower(unit_manager.unit_type[i]->side) == ToLower(players.side[ playerID ]))
				{
					weights[ i ].type = 0;

					if (unit_manager.unit_type[i]->canattack && unit_manager.unit_type[i]->BMcode && !unit_manager.unit_type[i]->commander)
					{
						if (unit_manager.unit_type[i]->canmove)
							weights[ i ].type |= AI_FLAG_ARMY;
						else
							weights[ i ].type |= AI_FLAG_DEFENSE;
					}

					if (unit_manager.unit_type[i]->Builder)
					{
						if (unit_manager.unit_type[i]->BMcode)
							weights[ i ].type |= AI_FLAG_BUILDER;
						else
							weights[ i ].type |= AI_FLAG_FACTORY;
					}

					if (unit_manager.unit_type[i]->MetalMake > 0.0f
						|| unit_manager.unit_type[i]->MakesMetal > 0.0f || unit_manager.unit_type[i]->ExtractsMetal > 0.0f)
					{
						weights[ i ].type |= AI_FLAG_METAL_P;
						weights[ i ].metal_s = (float)unit_manager.unit_type[i]->MetalStorage * 0.001f;
						weights[ i ].metal_p = (unit_manager.unit_type[i]->MetalMake + unit_manager.unit_type[i]->MakesMetal) * 10.0f
							+ 5000.0f * unit_manager.unit_type[i]->ExtractsMetal - (float)unit_manager.unit_type[i]->EnergyUse * 0.5f;
					}
					if (unit_manager.unit_type[i]->MetalStorage)
					{
						weights[ i ].type |= AI_FLAG_METAL_S;
						weights[ i ].metal_s = (float)unit_manager.unit_type[i]->MetalStorage * 0.001f;
						weights[ i ].metal_p = (unit_manager.unit_type[i]->MetalMake + unit_manager.unit_type[i]->MakesMetal) * 10.0f
							+ 5000.0f * unit_manager.unit_type[i]->ExtractsMetal - (float)unit_manager.unit_type[i]->EnergyUse * 0.5f;
					}
					if (unit_manager.unit_type[i]->EnergyMake || unit_manager.unit_type[i]->EnergyUse < 0.0f
						|| unit_manager.unit_type[i]->TidalGenerator || unit_manager.unit_type[i]->WindGenerator)
					{
						weights[ i ].type |= AI_FLAG_ENERGY_P;
						weights[ i ].energy_s = (float)unit_manager.unit_type[i]->EnergyStorage * 0.0001f;
						weights[ i ].energy_p = ((float)unit_manager.unit_type[i]->EnergyMake + (float)unit_manager.unit_type[i]->TidalGenerator
												 + (float)unit_manager.unit_type[i]->WindGenerator - (float)unit_manager.unit_type[i]->EnergyUse) * 0.1f;
					}
					if (unit_manager.unit_type[i]->EnergyStorage)
					{
						weights[ i ].type |= AI_FLAG_ENERGY_S;
						weights[ i ].energy_s = (float)unit_manager.unit_type[i]->EnergyStorage * 0.0001f;
						weights[ i ].energy_p = ((float)unit_manager.unit_type[i]->EnergyMake + (float)unit_manager.unit_type[i]->TidalGenerator
												 + (float)unit_manager.unit_type[i]->WindGenerator - (float)unit_manager.unit_type[i]->EnergyUse) * 0.1f;
					}
				}
				else
					weights[i].type = 0;
			}
			enemy_list.resize(players.count());
			for (unsigned int i = 0 ; i < players.count() ; ++i)
				enemy_list[i].clear();
		}

		if (unit_id == 0)
		{
			wip_builder_list.clear();
			wip_factory_list.clear();
			wip_army_list.clear();
			for (int i = 0; i < unit_manager.nb_unit; ++i) // reset things if needed
				weights[i].nb = 0;

			for (int i = 0; i < NB_AI_UNIT_TYPE; ++i)
				nb_units[i] = 0;

			for (int i = 0; i < 10; ++i)
				nb_enemy[i] = 0;
		}

		uint32 e;
		units.lock();
		for (e = unit_id ; e < units.nb_unit && e < unit_id + 10 ; ++e )
		{
			const uint32 i = units.idx_list[e];
			if (i >= units.max_unit)	continue;		// Error
			units.unlock();
			units.unit[i].lock();
			if ((units.unit[ i ].flags & 1) && units.unit[ i ].type_id >= 0)
			{
				if (units.unit[ i ].owner_id == playerID)
				{
					weights[ units.unit[ i ].type_id ].nb++;
					if (weights[ units.unit[ i ].type_id ].type & AI_FLAG_ARMY)
						wip_army_list.push_back( i );
					if (weights[ units.unit[ i ].type_id ].type & AI_FLAG_BUILDER)
						wip_builder_list.push_back( i );
					if (weights[ units.unit[ i ].type_id ].type & AI_FLAG_FACTORY)
						wip_factory_list.push_back( i );
				}
				else
				{
					nb_units[ AI_UNIT_TYPE_ENEMY ]++;
					nb_enemy[ units.unit[ i ].owner_id ]++;
					if (!enemy_table.contains(i))
					{
						enemy_list[ units.unit[ i ].owner_id ].push_back( WeightCoef( i, 0 ) );
						enemy_table.insert(i);
					}
				}
			}
			units.unit[ i ].unlock();
			units.lock();
		}
		units.unlock();
		unit_id = (e >= units.nb_unit) ? 0 : e;

		if (unit_id == 0)
		{
			builder_list.swap(wip_builder_list);
			factory_list.swap(wip_factory_list);
			army_list.swap(wip_army_list);

			wip_builder_list.clear();
			wip_factory_list.clear();
			wip_army_list.clear();
		}
	}



	void AiController::refresh_unit_weights()				// Refresh unit weights according to the unit scan and the orders weights
	{
		for (unsigned int i = 0; i < players.count(); ++i)
			std::sort(enemy_list[i].begin(), enemy_list[i].end());

		total_unit = 0;

		for (uint16 i = 0 ; i < unit_manager.nb_unit ; ++i )
			total_unit += weights[ i ].nb;

		const float total_unit_inv = 2.0f / float(total_unit + 1);

		for (uint16 i = 0 ; i < unit_manager.nb_unit ; ++i )
		{
			if (weights[ i ].type && !unit_manager.unit_type[i]->not_used)
			{
				if (weights[ i ].type & AI_FLAG_ARMY)
				{
					nb_units[ AI_UNIT_TYPE_ARMY ] += weights[ i ].nb;
					weights[ i ].w = (weights[ i ].w + order_weight[ ORDER_ARMY ] * 0.1f + 1.0f - (float)weights[ i ].nb * total_unit_inv) * expf( -0.1f * weights[ i ].w ) + 0.1f * order_weight[ ORDER_ARMY ];
				}
				if (weights[ i ].type & AI_FLAG_DEFENSE)
				{
					nb_units[ AI_UNIT_TYPE_DEFENSE ] += weights[ i ].nb;
					weights[ i ].w = (weights[ i ].w + order_weight[ ORDER_DEFENSE ] * 0.1f + 1.0f - (float)weights[ i ].nb * total_unit_inv) * expf( -0.1f * weights[ i ].w ) + 0.1f * order_weight[ ORDER_DEFENSE ];
				}
				if (weights[ i ].type & AI_FLAG_BUILDER)
				{
					nb_units[ AI_UNIT_TYPE_BUILDER ] += weights[ i ].nb;
					weights[ i ].w = (weights[ i ].w + order_weight[ ORDER_BUILDER ] * 0.1f + 1.0f - (float)weights[ i ].nb * total_unit_inv) * expf( -0.1f * weights[ i ].w ) + 0.1f * order_weight[ ORDER_BUILDER ];
				}
				if (weights[ i ].type & AI_FLAG_FACTORY)
				{
					nb_units[ AI_UNIT_TYPE_FACTORY ] += weights[ i ].nb;
					weights[ i ].w = (weights[ i ].w + order_weight[ ORDER_FACTORY ] * 0.1f + 1.0f - (float)weights[ i ].nb * total_unit_inv) * expf( -0.1f * weights[ i ].w ) + 0.1f * order_weight[ ORDER_FACTORY ];
				}
				if (weights[ i ].type & AI_FLAG_METAL_P)
				{
					nb_units[ AI_UNIT_TYPE_METAL ] += weights[ i ].nb;
					weights[ i ].w = (weights[ i ].w + order_weight[ ORDER_METAL_P ] * weights[ i ].metal_p * 0.1f + 1.0f - (float)weights[ i ].nb * total_unit_inv) * expf( -0.1f * weights[ i ].w ) + 0.1f * order_weight[ ORDER_METAL_P ] * weights[ i ].metal_p;
				}
				if (weights[ i ].type & AI_FLAG_METAL_S)
				{
					nb_units[ AI_UNIT_TYPE_METAL ] += weights[ i ].nb;
					weights[ i ].w = (weights[ i ].w + order_weight[ ORDER_METAL_S ] * weights[ i ].metal_s * 0.1f + 1.0f - (float)weights[ i ].nb * total_unit_inv) * expf( -0.1f * weights[ i ].w ) + 0.1f * order_weight[ ORDER_METAL_S ] * weights[ i ].metal_s;
				}
				if (weights[ i ].type & AI_FLAG_ENERGY_P)
				{
					nb_units[ AI_UNIT_TYPE_ENERGY ] += weights[ i ].nb;
					weights[ i ].w = (weights[ i ].w + order_weight[ ORDER_ENERGY_P ] * weights[ i ].energy_p * 0.1f + 1.0f - (float)weights[ i ].nb * total_unit_inv) * expf( -0.1f * weights[ i ].w ) + 0.1f * order_weight[ ORDER_ENERGY_P ] * weights[ i ].energy_p;
				}
				if (weights[ i ].type & AI_FLAG_ENERGY_S)
				{
					nb_units[ AI_UNIT_TYPE_ENERGY ] += weights[ i ].nb;
					weights[ i ].w = (weights[ i ].w + order_weight[ ORDER_ENERGY_S ] * weights[ i ].energy_s * 0.1f + 1.0f - (float)weights[ i ].nb * total_unit_inv) * expf( -0.1f * weights[ i ].w ) + 0.1f * order_weight[ ORDER_ENERGY_S ] * weights[ i ].energy_s;
				}
				if (weights[ i ].w < 0.0f)
					weights[ i ].w = 0.0f;
			}
			else
				weights[ i ].w = 0.0f;
			weights[ i ].o_w = weights[ i ].w;
		}
	}

	void AiController::think()				// La vrai fonction qui simule l'Intelligence Artificielle / The function that makes Artificial Intelligence work
	{
		srand( msec_timer );

		order_weight[ ORDER_METAL_P ] = Math::Max(0.0f, players.metal_u[ playerID ] - players.metal_t[playerID]) * 10.0f
			+ Math::Max(0.0f, players.metal[ playerID ] - float(players.metal_s[ playerID ] >> 1) ) * 0.01f;
		order_weight[ ORDER_ENERGY_P ] = Math::Max(0.0f, players.energy_u[ playerID ] * 2 - players.energy_t[playerID]) * 10.0f
			+ Math::Max(0.0f, float(players.energy_s[ playerID ] >> 1) - players.energy[ playerID ] ) * 0.1f;
		order_weight[ ORDER_METAL_S ] = Math::Max(0.0f, players.metal[ playerID ] - float(players.metal_s[ playerID] * 15 >> 4) ) * 0.001f;
		order_weight[ ORDER_ENERGY_S ] = Math::Max(0.0f, players.energy[ playerID ] - float(players.energy_s[ playerID ] * 15 >> 4) ) * 0.001f;

		order_weight[ ORDER_ARMY ] = float(nb_units[ AI_UNIT_TYPE_ENEMY ] - nb_units[ AI_UNIT_TYPE_ARMY ]) * 0.15f;
		order_weight[ ORDER_DEFENSE ] = float(nb_units[ AI_UNIT_TYPE_ENEMY ] - nb_units[ AI_UNIT_TYPE_DEFENSE ]) * 0.1f;

		for (size_t i = 0; i < 10; ++i)
		{
			order_attack[i] = nb_enemy[i] > 0 ? float(3 * nb_units[AI_UNIT_TYPE_ARMY] - nb_enemy[i]) * 0.1f : 0.0f;
			if (order_attack[i] < 0.0f)
				order_attack[i] = 0.0f;
			order_attack[i] = (1.0f - expf( -order_attack[i])) * 30.0f;
# ifdef AI_DEBUG
			LOG_DEBUG(LOG_PREFIX_AI << "Attack player " << i << " = " << order_attack[i]);
# endif
		}


# ifdef AI_DEBUG
		LOG_DEBUG(LOG_PREFIX_AI << "AI(" << (int)playerID << "," << msec_timer << ") thinking...");
# endif

		float sw[ 10000 ];			// Used to compute the units that are built

		{
			int player_target = -1;
			float best_weight = 15.0f;
			for (unsigned int e = 0 ; e < players.count(); ++e)				// Who can we attack ?
			{
				// Don't attack allies
				if (order_attack[e] > best_weight && !(players.team[playerID] & players.team[e]))
				{
					player_target = e;
					best_weight = order_attack[e];
				}
			}

			if (player_target >= 0 ) // If we've someone to attack
			{
				for (std::vector<uint32>::const_iterator i = army_list.begin() ; i != army_list.end() ; ++i ) // Give instructions to idle army units
				{
					suspend(1);
					units.unit[ *i ].lock();
					if ((units.unit[ *i ].flags & 1) && units.unit[ *i ].do_nothing_ai() )
					{
						int target_id = -1;
						while (!enemy_list[ player_target ].empty() && target_id == -1)
						{
							target_id = enemy_list[ player_target ].begin()->idx;
							if (!(units.unit[ target_id ].flags & 1) || units.unit[ target_id ].type_id < 0
								|| units.unit[ target_id ].type_id >= unit_manager.nb_unit || units.unit[ target_id ].owner_id != player_target )
							{
								enemy_table.remove(target_id);
								target_id = -1;
								enemy_list[ player_target ].pop_front();		// Remove what we've just read
							}
							else
							{
								if (units.unit[ target_id ].cloaked && !units.unit[ target_id ].is_on_radar(byte(1 << playerID) ) ) // This one is cloaked, not on radar
								{
									enemy_table.remove(target_id);
									target_id = -1;
									enemy_list[ player_target ].pop_front();		// Remove what we've just read
									continue;
								}
								enemy_list[ player_target ].begin()->c++;
							}
						}
						if (target_id >= 0 )
							units.unit[ *i ].set_mission( MISSION_ATTACK, &units.unit[ target_id ].Pos, false, 0, true, (&units.unit[ target_id ]), MISSION_FLAG_COMMAND_FIRE );
					}
					units.unit[ *i ].unlock();
				}
			}
		}

		for (std::vector<uint32>::const_iterator i = factory_list.begin() ; i != factory_list.end() ; ++i )	// Give instructions to idle factories
		{
			suspend(1);
			units.unit[ *i ].lock();
			if ((units.unit[ *i ].flags & 1) && units.unit[ *i ].do_nothing_ai() && unit_manager.unit_type[units.unit[*i].type_id]->nb_unit > 0)
			{
				const int list_size = unit_manager.unit_type[units.unit[*i].type_id]->nb_unit;
				const std::vector<short> &BuildList = unit_manager.unit_type[units.unit[*i].type_id]->BuildList;
				for (int e = 0 ; e < list_size ; ++e)
					sw[ e ] = (e > 0 ? sw[ e - 1 ] : 0.0f) + weights[ BuildList[ e ] ].w;
				int selected_idx = -1;
				const float selection = float(TA3D_RAND() % 1000000) * 0.000001f * sw[ list_size - 1 ];
				if (sw[ list_size - 1 ] > 0.1f)
					for (int e = 0 ; e < list_size ; ++e)
					{
						if (selection <= sw[ e ])
						{
							selected_idx = BuildList[e];
							break;
						}
					}
# ifdef AI_DEBUG
				LOG_DEBUG(LOG_PREFIX_AI << "AI(" << (int)playerID
						  << "," << msec_timer << ") -> factory " << *i << "building " << selected_idx);
# endif
				if (selected_idx >= 0)
				{
					units.unit[ *i ].add_mission( MISSION_BUILD, &units.unit[ *i ].Pos, false, selected_idx );
					weights[ selected_idx ].w *= 0.8f;
				}
			}
			units.unit[ *i ].unlock();
		}

		// Give instructions to idle builders
		for (std::vector<uint32>::const_iterator i = builder_list.begin() ; i != builder_list.end() ; ++i )
		{
			suspend(1);

			units.unit[ *i ].lock();
			if ((units.unit[ *i ].flags & 1) && units.unit[ *i ].do_nothing_ai() && unit_manager.unit_type[units.unit[*i].type_id]->nb_unit > 0)
			{
				const int list_size = unit_manager.unit_type[units.unit[*i].type_id]->nb_unit;
				const std::vector<short> &BuildList = unit_manager.unit_type[units.unit[*i].type_id]->BuildList;
				for (int e = 0 ; e < list_size ; ++e)
					sw[e] = (e > 0 ? sw[e - 1] : 0.0f) + weights[ BuildList[ e ] ].w;
				int selected_idx = -1;
				const float selection = float(TA3D_RAND() % 1000000) * 0.000001f * sw[ list_size - 1 ];
				if (sw[ list_size - 1 ] > 0.1f)
					for (int e = 0 ; e < list_size ; ++e)
					{
						if (selection <= sw[ e ])
						{
							selected_idx = BuildList[ e ];
							break;
						}
					}
				if (selected_idx >= 0)
				{
					Vector3D target = units.unit[ *i ].Pos;
					if (findBuildPlace(target, selected_idx, playerID, 5, 50))
					{
						if (unit_manager.unit_type[units.unit[*i].type_id]->BMcode)
							units.unit[ *i ].set_mission( MISSION_BUILD, &target, false, selected_idx );
						else
							units.unit[ *i ].add_mission( MISSION_BUILD, &target, false, selected_idx );
# ifdef AI_DEBUG
						LOG_DEBUG(LOG_PREFIX_AI << "AI(" << (int)playerID << "," << msec_timer
								  << ") -> builder " << *i << " building " << selected_idx);
# endif
						weights[ selected_idx ].w *= 0.8f;
					}
# ifdef AI_DEBUG
					else
						LOG_WARNING(LOG_PREFIX_AI << "AI(" << (int)playerID << "," << msec_timer
									<< ") -> builder " << *i << " building " << selected_idx << ": No build place found");
# endif
				}
			}
			units.unit[ *i ].unlock();
		}

		float factory_needed = 0.0f;
		float builder_needed = 0.0f;
		for (int i = 0 ; i < unit_manager.nb_unit ; ++i)	// Build required units
			if (weights[i].w >= weights[i].o_w)
			{
				if (weights[i].built_by.empty())
				{
					for (uint16 e = 0 ; e < unit_manager.nb_unit; ++e)
					{
						bool can_build = false;
						for (uint16 f = 0; f < unit_manager.unit_type[e]->nb_unit && !can_build ; ++f)
							can_build = unit_manager.unit_type[e]->BuildList[f] == i;
						if (can_build)
							weights[i].built_by.push_back(e);
					}
				}
				for (std::vector<uint16>::const_iterator e = weights[ i ].built_by.begin() ; e != weights[ i ].built_by.end() ; ++e )
				{
					if (weights[ *e ].type & AI_UNIT_TYPE_FACTORY)
						factory_needed += weights[ i ].w;
					if (weights[ *e ].type & AI_UNIT_TYPE_BUILDER)
						builder_needed += weights[ i ].w;
					weights[ *e ].w = weights[ *e ].w < weights[ i ].w ? (weights[ *e ].w + weights[ i ].w) * 0.5f : weights[ *e ].w;
				}
				weights[ i ].w *= 0.5f;				// Don't need to keep trying to build it if we can't
			}

		int n = 0;
		for(size_t i = 0 ; i < weights.size() ; ++i)
			n += weights[i].nb;
		const float populationLimit = std::max(0.0f, 1.0f - 2.0f * float(n) / (float)MAX_UNIT_PER_PLAYER);
		order_weight[ ORDER_FACTORY ] = factory_needed * populationLimit;
		order_weight[ ORDER_BUILDER ] = builder_needed * populationLimit;

		for(int i = 0 ; i < NB_ORDERS ; ++i)
		{
			if (order_weight[i] < 0.0f )
				order_weight[i] = 0.0f;
			order_weight[i] = (1.0f - expf( -order_weight[i])) * 30.0f;
		}

#ifdef AI_DEBUG
		printf("AI %d :\nARMY = %f\nFACTORY = %f\nDEFENSE = %f\nBUILDER = %f\nMETAL_P = %f\nMETAL_S = %f\nENERGY_P = %f\nENERGY_S = %f\n",
			   playerID, order_weight[ ORDER_ARMY ],order_weight[ ORDER_FACTORY ],order_weight[ ORDER_DEFENSE ],
			   order_weight[ ORDER_BUILDER ],order_weight[ ORDER_METAL_P ],order_weight[ ORDER_METAL_S ],
			   order_weight[ ORDER_ENERGY_P ],order_weight[ ORDER_ENERGY_S ]);
#endif

		return;							// Shortcut to prevent execution of this function because AI will be finished later
	}

	void	AiController::proc(void*)
	{
		thread_running = true;
		thread_ask_to_stop = false;
		uint32 speed = 10000U;
		uint32 timer = msec_timer;
		switch (AI_type)
		{
			case AI_TYPE_EASY    :speed = 10000U; break;
			case AI_TYPE_MEDIUM  :speed = 5000U;  break;
			case AI_TYPE_HARD    :speed = 2000U;  break;
			case AI_TYPE_BLOODY  :speed = 1000U;  break;
		}
		LOG_INFO(LOG_PREFIX_AI << "Started for player " << (int)playerID);
		while (!thread_ask_to_stop)
		{
			scan_unit();						// Look at the units

			if (unit_id == 0)	// When unit scanning is done
			{
				refresh_unit_weights();				// Refresh unit weights
				timer = msec_timer;
				think();
			}
			else
			{
				// Periodically think :) (gives orders to units)
				if (msec_timer - timer >= speed)
				{
					timer = msec_timer;
					think();
				}
			}

			float time_factor = units.apparent_timefactor;
			while ((Yuni::Math::Zero(time_factor) || lp_CONFIG->pause) && !thread_ask_to_stop)
			{
				time_factor = units.apparent_timefactor;
				suspend(10);
			}
			suspend(100 + (TA3D_RAND() % 100));
		}
		LOG_INFO(LOG_PREFIX_AI << "Stopped for player " << (int)playerID);
		thread_running = false;
		thread_ask_to_stop = false;
	}


	void AiController::signalExitThread()
	{
		LOG_INFO(LOG_PREFIX_AI << "Stopping for player " << (int)playerID << "...");
		thread_ask_to_stop = true;
		while (thread_running)
			suspend(1);
		thread_ask_to_stop = false;
	}


	void AiController::monitor()
	{
		if (!thread_running)
		{
			thread_running = true;
			start();
		}
	}

	void AiController::init()
	{
		thread_running = false;
		thread_ask_to_stop = false;

		name = "default ai";
		playerID = 0;
		unit_id = 0;
		AI_type = AI_TYPE_EASY;
		total_unit = 0;

		weights.clear();
		enemy_table.clear();

		builder_list.clear();
		factory_list.clear();
		army_list.clear();

		for (int i = 0 ; i < 10 ; ++i)
			nb_enemy[i] = 0;

		order_weight[ORDER_ARMY] = 0.5f;
		order_weight[ORDER_METAL_P] = 2.0f;
		order_weight[ORDER_ENERGY_P] = 3.0f;
		order_weight[ORDER_METAL_S] = 0.0f;
		order_weight[ORDER_ENERGY_S] = 0.0f;
		order_weight[ORDER_DEFENSE] = 0.0f;
		order_weight[ORDER_FACTORY] = 5.0f;
		order_weight[ORDER_BUILDER] = 5.0f;
	}

	void AiController::destroy()
	{
		destroyThread();

		builder_list.clear();
		factory_list.clear();
		army_list.clear();
		enemy_list.clear();

		playerID = 0;
		unit_id = 0;
		enemy_table.clear();
		weights.clear();
	}

	void AiController::changeName(const QString& newName)		// Change le nom de l'IA (conduit Ã  la crÃ©ation d'un nouveau fichier)
	{
		pMutex.lock();
		name = newName;
		pMutex.unlock();
	}

	void AiController::save()
	{
        Paths::MakeDir( Paths::Resources + "ai" );
        QString filename = Paths::Resources + "ai" + Paths::Separator + name + TA3D_AI_FILE_EXTENSION;
        QFile file(filename);
        file.open(QIODevice::WriteOnly);

        QDataStream stream(&file);
        stream << name;   // IA's name
		file.close();
	}


	void AiController::loadAI(const QString& filename, const int id)
	{
		File* file = VFS::Instance()->readFile(filename);

		// Length of the name
		const int l = file->getc();

		// Reading the name
		char* n = new char[l + 1];
        n[l] = 0;
		file->read(n, l);
		name = n;
		DELETE_ARRAY(n);

		delete file;
		playerID = id;
	}

	AiController::AiController() : builder_list(), factory_list(), army_list(), enemy_list()
	{
		init();
	}

	AiController::~AiController()
	{
		destroy();
	}

	void AiController::setPlayerID(int id)
	{
		lock();
		playerID = id;
		unlock();
	}

	int AiController::getPlayerID()
	{
		return playerID;
	}

	void AiController::setType(int type)
	{
		lock();
		AI_type = (byte)type;
		unlock();
	}

	int AiController::getType()
	{
		return AI_type;
	}

	bool AiController::findBuildPlace(Vector3D &target, int unit_idx, int playerID, int minRadius, int radius)
    {
        if (unit_idx < 0 || unit_idx >= unit_manager.nb_unit)
            return false;

		int px = (int)(target.x + (float)the_map->map_w_d + 4.0f) >> 3;
		int py = (int)(target.z + (float)the_map->map_h_d + 4.0f) >> 3;

        int spx = px;
        int spy = py;
        bool found = false;
        int best_metal = 0;
        int metal_stuff_id = -1;
		const bool extractor = unit_manager.unit_type[unit_idx]->ExtractsMetal > 0.0f;
		for (int r = minRadius ; r < radius && !found ; ++r) // Circular check
        {
			const int r2 = r * r;
			for (int y = (r >> 1) ; y <= r && !found ; ++y)
            {
				const int x = (int)(sqrtf(float(r2 - y * y)) + 0.5f);

				const int cx[] = { x, -x,  x, -x, y,  y, -y, -y };
				const int cy[] = { y,  y, -y, -y, x, -x,  x, -x };
                int rand_t[8];
                int rand_t2[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
                for (int e = 0 ; e < 8 ; ++e)
                {
					const int t = Math::RandomTable() % (8 - e);
                    rand_t[e] = rand_t2[t];
                    for (int f = t; f < 7 - e; ++f)
                        rand_t2[f] = rand_t2[f + 1];
                }

                for (int f = 0; f < 8; ++f)
                {
					const int e = rand_t[ f ];
					if (can_be_there_ai( px + cx[e], py + cy[e], unit_idx, playerID ))
                    {
                        int stuff_id = -1;
						const int metal_found = extractor ? the_map->check_metal( px + cx[e], py + cy[e], unit_idx, &stuff_id ) : 0;
                        if ((extractor && metal_found > best_metal) || !extractor)
                        {
                            // Prevent AI from filling a whole area with metal extractors
                            if (extractor && stuff_id == -1
								&& !can_be_there_ai( px + cx[e], py + cy[e], unit_idx, playerID, -1, true ))
                                continue;
                            spx = px + cx[e];
                            spy = py + cy[e];
                            if (metal_found > 0 && extractor)
                            {
                                best_metal = metal_found;
                                metal_stuff_id = stuff_id;
                                if (metal_stuff_id != -1)
                                    break;
                            }
                            else
                            {
                                found = true;
                                break;
                            }
                        }
                    }
                }
            }
        }

        px = spx;
        py = spy;
        found |= best_metal > 0;

        if (found && unit_idx >= 0)
        {
            if (metal_stuff_id >= 0)        // We have a valid metal patch
            {
                px = features.feature[ metal_stuff_id ].px;
                py = features.feature[ metal_stuff_id ].py;
            }
			target.x = float((px << 3) - the_map->map_w_d);
			target.z = float((py << 3) - the_map->map_h_d);
            target.y = Math::Max( the_map->get_max_rect_h((int)target.x,(int)target.z, unit_manager.unit_type[unit_idx]->FootprintX, unit_manager.unit_type[unit_idx]->FootprintZ ), the_map->sealvl);
            return true;
        }
        return false;
    }




} // namespace TA3D

