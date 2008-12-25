#include "signals.h"

function spawn_random( unit_type )
	rand_x = ( math.random() - 0.5 ) * ta3d_map_w()
	rand_z = ( math.random() - 0.5 ) * ta3d_map_h()
	unit_id = ta3d_create_unit( 1, unit_type, rand_x, rand_z )
	ta3d_attack( unit_id, player_com )
end

function main()
	if not cleared and ta3d_time() - timer >= 1 then
		cleared = true
		ta3d_cls()
	elseif not cleared then
		return 0
	end

	if end_signal ~= 0 and ta3d_time() - timer >= 5 then
		return end_signal
	elseif end_signal ~= 0 then
		return 0
	end

	if not ta3d_has_unit( local_player, ta3d_commander( local_player ) ) then
		ta3d_print( 280, 236, "DEFEAT!" )
		timer = ta3d_time()
		end_signal = SIGNAL_DEFEAT
	elseif current_level >= 3 and ta3d_annihilated( 1 ) then
		ta3d_play( "VICTORY2" )
		ta3d_draw_image( "gfx/victory.tga", 160, 140, 480, 340 )
		timer = ta3d_time()
		end_signal = SIGNAL_VICTORY
	end

	spawn = ta3d_time() - timer >= time_to_wait and time_to_wait >= 0

	if spawn then
		if current_level == 0 then
			for i = 0,15 do
				spawn_random( "ARMPW" )
			end
			time_to_wait = 60
		elseif current_level == 1 then
			for i = 0,30 do
				spawn_random( "ARMPW" )
			end
			time_to_wait = 130
		elseif current_level == 2 then
			for i = 0,60 do
				spawn_random( "ARMPW" )
			end
			time_to_wait = 120
		elseif current_level == 3 then
			for i = 0,30 do
				spawn_random( "ARMPW" )
				spawn_random( "ARMFLASH" )
			end
			time_to_wait = 120
		elseif current_level == 4 then
			for i = 0,30 do
				spawn_random( "ARMFIDO" )
				spawn_random( "ARMMAV" )
			end
			time_to_wait = -1
		end
		timer = ta3d_time()
		current_level = current_level + 1
	end

	ta3d_cls()
	if time_to_wait >= 0 then
		ta3d_print( 128, 32, string.format( "%d sec. left", time_to_wait + timer - ta3d_time() ) )
	else
		ta3d_give_energy( 1, 10000 )
	end

	return 0
end

local_player = ta3d_local_player()

ta3d_print( 296, 236, "Welcome" )
timer = ta3d_time()
cleared = false
end_signal = 0

player_com = ta3d_create_unit( local_player, ta3d_commander( local_player ), ta3d_start_x( local_player ), ta3d_start_z( local_player ) )

ta3d_set_cam_pos( local_player, ta3d_unit_x( player_com ), ta3d_unit_z( player_com ) + 64 )

ta3d_clf()
ta3d_init_res()

time_to_wait = 60
current_level = 0
