#include "signals.h"

function main()
	if not cleared and ta3d_time() - timer >= 1 then
		cleared = true
		ta3d_cls()
	elseif not cleared then
		return 0
	end

	for current_player = 0, ta3d_nb_players() - 1 do

		if end_signal[ current_player ] ~= 0 and ta3d_time() - player_timer[ current_player ] >= 5 then
			if not signal_sent[ current_player ] then
				ta3d_send_signal( current_player, end_signal[ current_player ] )
				signal_sent[ current_player ] = true
			end
		elseif end_signal[ current_player ] == 0 then

			win = true
			for i = 0, ta3d_nb_players() - 1 do
				if not ta3d_allied( i, current_player ) then
					if not ta3d_annihilated( i ) then
						win = false
					end
				end
			end

			if win then
				if ta3d_annihilated( current_player ) then
					ta3d_print_for( 280, 236, "MATCH NUL!", current_player )
					player_timer[ current_player ] = ta3d_time()
					end_signal[ current_player ] = SIGNAL_NUL
				else
					ta3d_play_for( "VICTORY2", current_player )
					ta3d_draw_image_for( "gfx/victory.tga", 160, 140, 480, 340, current_player )
					player_timer[ current_player ] = ta3d_time()
					end_signal[ current_player ] = SIGNAL_VICTORY
				end
			elseif ta3d_annihilated( current_player ) then
					ta3d_print_for( 288, 236, "DEFAITE!", current_player )
					player_timer[ current_player ] = ta3d_time()
					end_signal[ current_player ] = SIGNAL_DEFEAT
			end

		end

	end

	return 0
end

ta3d_print( 296, 236, "Welcome" )
timer = ta3d_time()
cleared = false

end_signal = {}
signal_sent = {}
player_timer = {}
for i = 0, 9 do
	end_signal[i] = 0
	signal_sent[i] = false
	player_timer[i] = ta3d_time()
end

for i = 0, ta3d_nb_players() - 1 do
	player_com = ta3d_create_unit( i, ta3d_commander( i ), ta3d_start_x( i ), ta3d_start_z( i ) )

	ta3d_set_cam_pos( i, ta3d_unit_x( player_com ), ta3d_unit_z( player_com ) + 64 )
end
ta3d_clf()
ta3d_init_res()
