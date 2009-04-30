-- Arm Construction Vehicle

piece( "base", "beam", "arm", "door1", "door2", "nano", "plate", "turret" )

buildheading = 0

SMOKEPIECE1     = base
ANIM_VARIABLE   = true

#include "StateChg.lh"
#include "smokeunit.lh"
#include "exptype.lh"

function activatescr()
    turn( door1, z_axis, -90, 90 )
    turn( door2, z_axis, 90, 90 )
    
    wait_for_turn( door1, z_axis )
    wait_for_turn( door2, z_axis )
    
    turn( arm, x_axis, 90, 90 )
    turn( nano, x_axis, -90, 90 )

    wait_for_turn( arm, x_axis )
    wait_for_turn( nano, x_axis )
end

function deactivatescr()
    turn( arm, x_axis, 0, 90 )
    turn( nano, x_axis, 0, 90 )

    wait_for_turn( arm, x_axis )
    wait_for_turn( nano, x_axis )

    turn( door1, z_axis, 0, 90 )
    turn( door2, z_axis, 0, 90 )
    
    wait_for_turn( door1, z_axis )
    wait_for_turn( door2, z_axis )
end

function Go()
	activatescr()
	turn( turret, y_axis, buildheading, 160 )
	wait_for_turn( turret, y_axis )
	set( INBUILDSTANCE, true )
end

function Stop()
	set( INBUILDSTANCE, false )
	turn( turret, y_axis, 0, 160 )
	wait_for_turn( turret, y_axis )
	deactivatescr()
end

ACTIVATECMD     = Go
DEACTIVATECMD   = Stop

function Create()
	buildheading = 0
	InitState()
	start_script( SmokeUnit )
end

function Activate()
	start_script( RequestState, ACTIVE )
end

function Deactivate()
	start_script( RequestState, INACTIVE )
end

function StartBuilding(heading)
	buildheading = heading * TA2DEG
	start_script( RequestState, ACTIVE )
end

function StopBuilding()
	start_script( RequestState, INACTIVE )
end

function QueryNanoPiece()
	return beam
end

function TargetHeading( heading )
	buildheading = -heading * TA2DEG
end

function SweetSpot()
	return base
end

function Killed( severity )
	if severity <= 25 then
		explode( arm, BITMAPONLY + BITMAP1 )
		explode( base, BITMAPONLY + BITMAP2 )
		explode( beam, BITMAPONLY + BITMAP3 )
		explode( door1, BITMAPONLY + BITMAP4 )
		explode( door2, BITMAPONLY + BITMAP5 )
		explode( nano, BITMAPONLY + BITMAP1 )
		explode( plate, BITMAPONLY + BITMAP2 )
		explode( turret, BITMAPONLY + BITMAP3 )
		return 1
	end

	if severity <= 50 then
		explode( arm, FALL + BITMAP1 )
		explode( base, BITMAPONLY + BITMAP2 )
		explode( beam, FALL + BITMAP3 )
		explode( door1, BITMAPONLY + BITMAP4 )
		explode( door2, BITMAPONLY + BITMAP5 )
		explode( nano, SHATTER + BITMAP1 )
		explode( plate, BITMAPONLY + BITMAP2 )
		explode( turret, FALL + BITMAP3 )
		return 2
	end

	if severity <= 99 then
		explode( arm, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
		explode( base, BITMAPONLY + BITMAP2 )
		explode( beam, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
		explode( door1, BITMAPONLY + BITMAP4 )
		explode( door2, BITMAPONLY + BITMAP5 )
		explode( nano, SHATTER + BITMAP1 )
		explode( plate, BITMAPONLY + BITMAP2 )
		explode( turret, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
		return 3
	end

	explode( arm, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
	explode( base, BITMAPONLY + BITMAP2 )
	explode( beam, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
	explode( door1, BITMAPONLY + BITMAP4 )
	explode( door2, BITMAPONLY + BITMAP5 )
	explode( nano, SHATTER + EXPLODE_ON_HIT + BITMAP1 )
	explode( plate, BITMAPONLY + BITMAP2 )
	explode( turret, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
	return 3
end
