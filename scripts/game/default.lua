#include "signals.lh"

killed = {}
for i = 0, nb_players() - 1 do
	killed[ i ] = false
end

text_print( 296, 236, "Welcome" )
timer = time()
cleared = false

end_signal = {}
signal_sent = {}
player_timer = {}
for i = 0, 9 do
	end_signal[i] = 0
	signal_sent[i] = false
	player_timer[i] = time()
end

for i = 0, nb_players() - 1 do
	player_com = create_unit( i, commander( i ), start_x( i ), start_z( i ) )

	set_cam_pos( i, unit_x( player_com ), unit_z( player_com ) + 64 )
end
clf()
init_res()

while true do
    yield()
	if not cleared and time() - timer >= 1 then
		cleared = true
		cls()
	elseif not cleared then
    else
	    for current_player = 0, nb_players() - 1 do

		    if end_signal[ current_player ] ~= 0 and time() - player_timer[ current_player ] >= 5 then
			    if not signal_sent[ current_player ] then
				    send_signal( current_player, end_signal[ current_player ] )
				    signal_sent[ current_player ] = true
			    end
		    elseif end_signal[ current_player ] == 0 then

			    win = true
			    for i = 0, nb_players() - 1 do
				    if not allied( i, current_player ) then
					    if has_unit( i, commander( i ) ) then
						    win = false
					    elseif not killed[ i ] then								-- when a player dies, kill all its units
						    killed[ i ] = true
						    for e = 0, get_max_unit_number() - 1 do
							    if get_unit_owner( e ) == i then
								    kill_unit( e )
							    end
						    end
					    end
				    end
			    end

			    if win then
				    if not has_unit( current_player, commander( current_player ) ) then
					    text_print_for( 280, 236, "MATCH NUL!", current_player )
					    player_timer[ current_player ] = time()
					    end_signal[ current_player ] = SIGNAL_NUL
				    else
					    play_for( "VICTORY2", current_player )
					    draw_image_for( "gfx/victory.tga", 160, 140, 480, 340, current_player )
					    player_timer[ current_player ] = time()
					    end_signal[ current_player ] = SIGNAL_VICTORY
				    end
			    elseif not has_unit( current_player, commander( current_player ) ) then
					    text_print_for( 288, 236, "DEFAITE!", current_player )
					    player_timer[ current_player ] = time()
					    end_signal[ current_player ] = SIGNAL_DEFEAT
			    end
		    end
	    end
    end
end

