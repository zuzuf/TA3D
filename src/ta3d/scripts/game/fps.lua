#include "signals.lh"

killed = {}
for i = 0, nb_players() - 1 do
    killed[ i ] = false
end

text_print( 296, 236, "Good luck commander" )
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

player_com = {}
for i = 0, nb_players() - 1 do
    player_com[i] = create_unit( i, commander( i ), start_x( i ), start_z( i ) )
end

clf()
init_res()

local I = vector()
I.x, I.y, I.z = 0, 0, 1

while true do
    yield()
    for i = 0, nb_players() - 1 do
        local camera = { pos = vector(), dir = vector(), mode = true }
        camera.pos = unit_pos( player_com[i] )
        camera.dir = unit_angle( player_com[i] )
        local dir = unit_piece_dir( player_com[i], "head" )
        local angle_y = vangle(I, dir)
        if dir.x < 0 then
            angle_y = -angle_y
        end
        camera.dir.y = angle_y + 180
        camera.pos = camera.pos + unit_piece_pos( player_com[i], "head" ) + 5 * dir

        set_cam( i, camera )
    end

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
                        elseif not killed[ i ] then                             -- when a player dies, kill all its units
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
                        draw_image_for( "gfx/victory.png", 145, 190, 495, 290, current_player )
                        player_timer[ current_player ] = time()
                        end_signal[ current_player ] = SIGNAL_VICTORY
                    end
                elseif not has_unit( current_player, commander( current_player ) ) then
                        draw_image_for( "gfx/defeat.png", 145, 190, 495, 290, current_player )
                        player_timer[ current_player ] = time()
                        end_signal[ current_player ] = SIGNAL_DEFEAT
                end
            end
        end
    end
end

 
