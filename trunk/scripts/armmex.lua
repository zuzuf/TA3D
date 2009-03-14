-- Arm Metal Extractor Script

piece( "base", "arms" )

SMOKEPIECE1 = base

#include "StateChg.h"
#include "smokeunit.h"
#include "exptype.h"

spinspeed = 0
spinacc = 0.5
spindec = 1.0

function Go()
	spin( arms, y_axis, spinspeed, spinacc )
end

function Stop()
	stop_spin( arms, y_axis, spindec )
	wait_for_turn( arms, y_axis )
end

ACTIVATECMD = Go
DEACTIVATECMD = Stop

function Create()
	spinspeed = 0
	spinacc = 0.5
	spindec = 1.0
	dont_shade( arms )
	dont_cache( arms )
	InitState()
	start_script( SmokeUnit )
end

function Activate()
	start_script( RequestState, ACTIVE )
end

function Deactivate()
	start_script( RequestState, INACTIVE )
end

function SweetSpot()
	return base
end

function SetSpeed(the_speed)
	spinspeed = the_speed * 0.25
end

function Killed( severity )
	if severity <= 25 then
		explode( arms, BITMAPONLY + BITMAP1 )
		explode( base, BITMAPONLY + BITMAP2 )
		return 1
	end

	if severity <= 50 then
		explode( arms, SHATTER + BITMAP1 )
		explode( base, BITMAPONLY + BITMAP2 )
		return 2
	end

	if severity <= 99 then
		explode( arms, SHATTER + EXPLODE_ON_HIT + BITMAP1 )
		explode( base, BITMAPONLY + BITMAP2 )
		return 3
	end

	explode( arms, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
	explode( base, SHATTER + EXPLODE_ON_HIT + BITMAP2 )
	return 3
end
