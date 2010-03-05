#include "signals.lh"

function spawn_random( unit_type )
	rand_x = ( math.random() - 0.5 ) * map_w()
	rand_z = ( math.random() - 0.5 ) * map_h()
	unit_id = create_unit( 1, unit_type, rand_x, rand_z )
	attack( unit_id, player_com )
end

local_player = local_player()

text_print( 296, 236, "Welcome" )
timer = time()
cleared = false
end_signal = 0

player_com = create_unit( local_player, commander( local_player ), start_x( local_player ), start_z( local_player ) )

set_cam_pos( local_player, unit_x( player_com ), unit_z( player_com ) + 64 )

clf()
init_res()

time_to_wait = 60
current_level = 0

while true do
    yield()
	if not cleared and time() - timer >= 1 then
		cleared = true
		cls()
	elseif not cleared then
	else
	    if end_signal ~= 0 and time() - timer >= 5 then
		    signal( end_signal )
	    elseif end_signal ~= 0 then
	    else
	        if not has_unit( local_player, commander( local_player ) ) then
		        draw_image( "gfx/defeat.png", 145, 190, 495, 290 )
		        timer = time()
		        end_signal = SIGNAL_DEFEAT
	        elseif current_level >= 3 and annihilated( 1 ) then
		        play( "VICTORY2" )
		        draw_image( "gfx/victory.png", 145, 190, 495, 290 )
		        timer = time()
		        end_signal = SIGNAL_VICTORY
	        end

	        spawn = time() - timer >= time_to_wait and time_to_wait >= 0

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
		        timer = time()
		        current_level = current_level + 1
	        end

	        cls()
	        if time_to_wait >= 0 then
		        text_print( 128, 32, string.format( "%d sec. left", time_to_wait + timer - time() ) )
	        else
		        give_energy( 1, 10000 )
	        end
	    end
    end
end
