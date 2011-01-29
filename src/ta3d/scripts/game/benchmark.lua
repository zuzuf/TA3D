#include "signals.lh"

function spawn_random( pid )
	local rand_x = ( math.random() - 0.5 ) * map_w()
	local rand_z = ( math.random() - 0.5 ) * map_h()
	if create_unit( pid, -1, rand_x, rand_z ) == -1 then
		sleep(10)
	end
end

local_player = local_player()

text_print( -320, -240, "Starting benchmark!" )

set_cam_pos( local_player, start_x( local_player ), start_z( local_player ) + 64 )

clf()
init_res()

sleep(2)

cls()

player_count = nb_players() - 1

while true do
    sleep(0.01)
	for player_id = 0,player_count do
	  spawn_random( player_id )
	end
end
