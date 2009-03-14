-- Arm Plasma Defense Battery

piece( "flare1", "flare2", "base", "turret", "sleeves", "barrel1", "barrel2" )

fire = 0

SIG_AIM     = 2
SMOKEPIECE1 = base

#include "smokeunit.h"
#include "EXPtype.h"

function Create()
	hide( flare1 )
	hide( flare2 )
	dont_cache( flare1 )
	dont_cache( flare2 )
	dont_cache( barrel1 )
	dont_cache( barrel2 )
	dont_cache( sleeves )
	dont_cache( turret )
	fire = 0
	start_script( SmokeUnit )
end

function AimPrimary(heading,pitch)
	set_script_value("AimPrimary", false)

	signal( SIG_AIM )
	set_signal_mask( SIG_AIM )
	
	heading = heading * TA2DEG
	pitch = pitch * TA2DEG

	turn( turret, y_axis, heading, 30 )
	turn( sleeves, x_axis, -pitch, 45 )
	wait_for_turn( turret, y_axis )
	wait_for_turn( sleeves, x_axis )

	set_script_value("AimPrimary", true)
end

function FirePrimary()
	if fire == 0 then
		move_piece_now( barrel1, z_axis, -2.5 )
		show( flare1 )
		sleep( 0.15 )
		hide( flare1 )
		move( barrel1, z_axis, 0, 1 )
		fire = 1
	else
		move_piece_now( barrel2, z_axis, -2.5 )
		show( flare2 )
		sleep( 0.15 )
		hide( flare2 )
		move( barrel2, z_axis, 0, 1 )
		fire = 0
	end
end

function QueryPrimary()
    if fire == 0 then
    	return flare1
    end
    return flare2
end

function AimFromPrimary()
	return turret
end

function SweetSpot()
	return base
end

function Killed( severity )
	hide( flare1 )
	hide( flare2 )
	if severity <= 25 then
		explode( barrel1, BITMAPONLY + BITMAP1 )
		explode( barrel2, BITMAPONLY + BITMAP2 )
		explode( base, BITMAPONLY + BITMAP3 )
		explode( flare1, BITMAPONLY + BITMAP4 )
		explode( flare1, BITMAPONLY + BITMAP5 )
		explode( sleeves, BITMAPONLY + BITMAP1 )
		explode( turret, BITMAPONLY + BITMAP2 )
		return 1
	end

	if severity <= 50 then
		explode( barrel1, FALL + BITMAP1 )
		explode( barrel2, FALL + BITMAP2 )
		explode( base, BITMAPONLY + BITMAP3 )
		explode( flare1, FALL + BITMAP4 )
		explode( flare1, FALL + BITMAP5 )
		explode( sleeves, SHATTER + BITMAP1 )
		explode( turret, BITMAPONLY + BITMAP2 )
		return 2
	end

	if severity <= 99 then
		explode( barrel1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
		explode( barrel2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
		explode( base, BITMAPONLY + BITMAP3 )
		explode( flare1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4 )
		explode( flare1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP5 )
		explode( sleeves, SHATTER + BITMAP1 )
		explode( turret, BITMAPONLY + BITMAP2 )
		return 3
	end

	explode( barrel1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
	explode( barrel2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
	explode( base, BITMAPONLY + BITMAP3 )
	explode( flare1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4 )
	explode( flare1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP5 )
	explode( sleeves, SHATTER + EXPLODE_ON_HIT + BITMAP1 )
	explode( turret, BITMAPONLY + BITMAP2 )
	return 3
end

