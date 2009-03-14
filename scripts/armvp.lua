-- Arm Vehicle Plant Script

piece( "base", "pad", "beam1", "beam2", "doo2", "door1", "nano1", "nano2", "plate1", "plate2", "post1", "post2", "side1", "side2" )

SIG_ACTIVATE = 2
SMOKEPIECE1 = base
ANIM_VARIABLE = true

#include "StateChg.h"
#include "smokeunit.h"
#include "exptype.h"
#include "yard.h"

function activatescr()
    turn_object( side1, z_axis, -90, 60 )
    turn_object( side2, z_axis, 90, 60 )

    wait_for_turn( side1, z_axis )
    wait_for_turn( side2, z_axis )

    turn_object( door1, z_axis, -90, 60 )
    turn_object( doo2, z_axis, 90, 60 )

    wait_for_turn( door1, z_axis )
    wait_for_turn( doo2, z_axis )
    
    move_object( door1, x_axis, 10, 10 )
    move_object( doo2, x_axis, -10, 10 )

    move_object( plate1, x_axis, -8, 8 )
    move_object( plate2, x_axis, 7.5, 7.5 )

    wait_for_move( plate1, x_axis )
    wait_for_move( plate2, x_axis )
    wait_for_move( door1, x_axis )
    wait_for_move( doo2, x_axis )
end

function deactivatescr()
    move_object( door1, x_axis, 0, 10 )
    move_object( doo2, x_axis, 0, 10 )

    move_object( plate1, x_axis, 0, 8 )
    move_object( plate2, x_axis, 0, 7.5 )

    wait_for_move( plate1, x_axis )
    wait_for_move( plate2, x_axis )
    wait_for_move( door1, x_axis )
    wait_for_move( doo2, x_axis )

    turn_object( door1, z_axis, 0, 60 )
    turn_object( doo2, z_axis, 0, 60 )

    wait_for_turn( door1, z_axis )
    wait_for_turn( doo2, z_axis )

    turn_object( side1, z_axis, 0, 60 )
    turn_object( side2, z_axis, 0, 60 )

    wait_for_turn( side1, z_axis )
    wait_for_turn( side2, z_axis )
end

function Go()
	dont_cache( doo2 )
	dont_cache( door1 )
	dont_cache( nano1 )
	dont_cache( nano2 )
	dont_cache( pad )
	dont_cache( plate1 )
	dont_cache( plate2 )
	dont_cache( post1 )
	dont_cache( post2 )
	dont_cache( side1 )
	dont_cache( side2 )
	activatescr()
	OpenYard()
	set( INBUILDSTANCE, true )
end

function Stop()
	set( INBUILDSTANCE, false )
	CloseYard()
	deactivatescr()
	cache( doo2 )
	cache( door1 )
	cache( nano1 )
	cache( nano2 )
	cache( pad )
	cache( plate1 )
	cache( plate2 )
	cache( post1 )
	cache( post2 )
	cache( side1 )
	cache( side2 )
end

ACTIVATECMD = Go
DEACTIVATECMD = Stop

function Create()
	spray = 0
	InitState()
	start_script( SmokeUnit )
end

function QueryNanoPiece(piecenum)
	if spray == 0 then
		piecenum = beam1
	else
		piecenum = beam2
	end
	spray = (spray + 1) % 2
	return piecenum
end

function Activate()
	signal( SIG_ACTIVATE )
	start_script( RequestState, ACTIVE )
end

function Deactivate()
	signal( SIG_ACTIVATE )
	set_signal_mask( SIG_ACTIVATE )
	sleep( 5 )
	set_signal_mask( 0 )
	start_script( RequestState, INACTIVE )
end

function StartBuilding()
	spin( pad, y_axis, 30 )
end

function StopBuilding()
	stop_spin( pad, y_axis )
end

function QueryBuildInfo(piecenum)
	return pad
end

function SweetSpot(piecenum)
	return base
end

function Killed( severity, corpsetype )
	if severity <= 25 then
		corpsetype = 1
		explode( base,	BITMAPONLY + BITMAP1 )
		explode( doo2,	BITMAPONLY + BITMAP4 )
		explode( door1,	BITMAPONLY + BITMAP5 )
		explode( nano1,	BITMAPONLY + BITMAP1 )
		explode( nano2,	BITMAPONLY + BITMAP2 )
		explode( pad,	BITMAPONLY + BITMAP3 )
		explode( plate1,	BITMAPONLY + BITMAP4 )
		explode( plate2,	BITMAPONLY + BITMAP5 )
		explode( post1,	BITMAPONLY + BITMAP1 )
		explode( post2,	BITMAPONLY + BITMAP2 )
		explode( side1,	BITMAPONLY + BITMAP3 )
		explode( side2,	BITMAPONLY + BITMAP4 )
		return 0
	end

	if severity <= 50 then
		corpsetype = 2
		explode( base,	BITMAPONLY + BITMAP1 )
		explode( doo2,	FALL + BITMAP4 )
		explode( door1,	BITMAPONLY + BITMAP5 )
		explode( nano1,	BITMAPONLY + BITMAP1 )
		explode( nano2,	FALL + BITMAP2 )
		explode( pad,	BITMAPONLY + BITMAP3 )
		explode( plate1,	SHATTER + BITMAP4 )
		explode( plate2,	BITMAPONLY + BITMAP5 )
		explode( post1,	FALL + BITMAP1 )
		explode( post2,	FALL + BITMAP2 )
		explode( side1,	BITMAPONLY + BITMAP3 )
		explode( side2,	BITMAPONLY + BITMAP4 )
		return 0
	end

	if severity <= 99 then
		corpsetype = 3
		explode( base,	BITMAPONLY + BITMAP1 )
		explode( doo2,	FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4 )
		explode( door1,	BITMAPONLY + BITMAP5 )
		explode( nano1,	BITMAPONLY + BITMAP1 )
		explode( nano2,	FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
		explode( pad,	BITMAPONLY + BITMAP3 )
		explode( plate1,	SHATTER + BITMAP4 )
		explode( plate2,	SHATTER + BITMAP5 )
		explode( post1,	FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
		explode( post2,	FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
		explode( side1,	BITMAPONLY + BITMAP3 )
		explode( side2,	BITMAPONLY + BITMAP4 )
		return 0
	end

	corpsetype = 3
	explode( base,	BITMAPONLY + BITMAP1 )
	explode( doo2,	FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4 )
	explode( door1,	BITMAPONLY + BITMAP5 )
	explode( nano1,	BITMAPONLY + BITMAP1 )
	explode( nano2,	FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
	explode( pad,	BITMAPONLY + BITMAP3 )
	explode( plate1,    SHATTER + EXPLODE_ON_HIT + BITMAP4 )
	explode( plate2,	SHATTER + EXPLODE_ON_HIT + BITMAP5 )
	explode( post1,	FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
	explode( post2,	FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
	explode( side1,	BITMAPONLY + BITMAP3 )
	explode( side2,	BITMAPONLY + BITMAP4 )
	return 0
end

