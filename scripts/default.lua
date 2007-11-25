#include "signals.h"

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

	win = true
	for i = 0, ta3d_nb_players() - 1 do
		if i ~= local_player then
			if ta3d_has_unit( i, ta3d_commander( i ) ) then
				win = false
			elseif not killed[ i ] then								-- when a player dies, kill all its units
				killed[ i ] = true
				for e = 0, ta3d_get_max_unit_number() - 1 do
					if ta3d_get_unit_owner( e ) == i then
						ta3d_kill_unit( e )
					end
				end
			end
		end
	end

	if win then
		if not ta3d_has_unit( local_player, ta3d_commander( local_player ) ) then
			ta3d_print( 280, 236, "MATCH NUL!" )
			timer = ta3d_time()
			end_signal = SIGNAL_NUL
		else
			ta3d_play( "VICTORY2" )
			ta3d_draw_image( "gfx/victory.tga", 160, 140, 480, 340 )
			timer = ta3d_time()
			end_signal = SIGNAL_VICTORY
		end
	elseif not ta3d_has_unit( local_player, ta3d_commander( local_player ) ) then
			ta3d_print( 288, 236, "DEFAITE!" )
			timer = ta3d_time()
			end_signal = SIGNAL_DEFEAT
	end
	return 0
end

killed = {}
for i = 0, ta3d_nb_players() - 1 do
	killed[ i ] = false
end

local_player = ta3d_local_player()

ta3d_print( 296, 236, "Welcome" )
timer = ta3d_time()
cleared = false
end_signal = 0

for i = 0, ta3d_nb_players() - 1 do
	player_com = ta3d_create_unit( i, ta3d_commander( i ), ta3d_start_x( i ), ta3d_start_z( i ) )

	ta3d_set_cam_pos( i, ta3d_unit_x( player_com ), ta3d_unit_z( player_com ) + 64 )
end
ta3d_clf()
ta3d_init_res()

