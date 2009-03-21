-- SmokeUnit.h -- Process unit smoke when damaged

#include "SFXtype.h"
#include "EXPtype.h"

-- Figure out how many smoking pieces are defined

if SMOKEPIECE4 ~= nil then
    NUM_SMOKE_PIECES = 4
elseif SMOKEPIECE3 ~= nil then
    NUM_SMOKE_PIECES = 3
elseif SMOKEPIECE2 ~= nil then
    NUM_SMOKE_PIECES = 2
else
    NUM_SMOKE_PIECES = 1
    if SMOKEPIECE1 == nil then
        SMOKEPIECE1 = SMOKEPIECE
    end
end

function SmokeUnit()
    local healthpercent
    local sleeptime
    local smoketype

    -- Wait until the unit is actually built
    while get(BUILD_PERCENT_LEFT) > 0 do
        sleep(0.4)
    end

    -- Smoke loop
    while true do
        -- How is the unit doing?
        healthpercent = get( HEALTH )

        if healthpercent < 66 then
            -- Emit a puff of smoke

            smoketype = SFXTYPE_BLACKSMOKE

            if math.random( 1, 66 ) < healthpercent then
                smoketype = SFXTYPE_WHITESMOKE
            end

             -- Figure out which piece the smoke will emit from, and spit it out

            if NUM_SMOKE_PIECES == 1 then
                emit_sfx( smoketype, SMOKEPIECE1 )
            else
                local choice = math.random( 1, NUM_SMOKE_PIECES )

                if choice == 1 then
                    emit_sfx( smoketype, SMOKEPIECE1 )
                elseif choice == 2 then
                    emit_sfx( smoketype, SMOKEPIECE2 )
                elseif NUM_SMOKE_PIECES >= 3 then
                    if choice == 3 then
                        emit_sfx( smoketype, SMOKEPIECE3 )
                        if NUM_SMOKE_PIECES >= 4 and choice == 4 then
                            emit_sfx( smoketype, SMOKEPIECE4 )
                        end
                    end
                end
            end
        end

        -- Delay between puffs

        sleeptime = healthpercent * 0.05
        if sleeptime < 0.2 then
            sleeptime = 0.2    -- Fastest rate is five times per second
        end

        sleep( sleeptime )
    end
end
