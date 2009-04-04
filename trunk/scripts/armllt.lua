-- Arm Laser Defense Battery

piece( "base", "flare", "turret", "sleeve", "barrel" )

SIG_AIM	        = 2
SMOKEPIECE1     = base

#include "smokeunit.lh"
#include "EXPtype.lh"

function Create()
	hide( flare )
	dont_cache( flare )
	dont_cache( turret )
	dont_cache( sleeve )
	dont_cache( barrel )
	dont_shade( flare )
	dont_shade( turret )
	dont_shade( sleeve )
	dont_shade( barrel )
	start_script( SmokeUnit )
end

function AimPrimary(heading, pitch)
    set_script_value("AimPrimary", false)
	signal( SIG_AIM )
	set_signal_mask( SIG_AIM )

	heading = heading * TA2DEG
	pitch = pitch * TA2DEG

	turn( turret, y_axis, heading, 300 )
	turn( sleeve, x_axis, -pitch, 200 )
	wait_for_turn( turret, y_axis )
	wait_for_turn( sleeve, x_axis )
    set_script_value("AimPrimary", true)
end

function FirePrimary()
	show( flare )
	sleep( 0.15 )
	hide( flare )
end

function AimFromPrimary()
	return turret
end

function QueryPrimary()
	return flare
end

function SweetSpot()
	return base
end

function Killed( severity )
	hide( flare )
	if severity <= 25 then
		explode( barrel, BITMAPONLY + BITMAP1 )
		explode( base, BITMAPONLY + BITMAP2 )
		explode( flare, BITMAPONLY + BITMAP3 )
		explode( sleeve, BITMAPONLY + BITMAP4 )
		explode( turret, BITMAPONLY + BITMAP5 )
		return 1
	end

	if severity <= 50 then
		explode( barrel, FALL + BITMAP1 )
		explode( base, BITMAPONLY + BITMAP2 )
		explode( flare, FALL + BITMAP3 )
		explode( sleeve, SHATTER + BITMAP4 )
		explode( turret, FALL + BITMAP5 )
		return 2
	end

	if severity <= 99 then
		explode( barrel, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
		explode( base, BITMAPONLY + BITMAP2 )
		explode( flare, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
		explode( sleeve, SHATTER + BITMAP4 )
		explode( turret, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP5 )
		return 3
	end

	explode( barrel, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
	explode( base, BITMAPONLY + BITMAP2 )
	explode( flare, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
	explode( sleeve, SHATTER + EXPLODE_ON_HIT + BITMAP4 )
	explode( turret, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP5 )
	return 3
end

