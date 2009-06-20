-- Arm Laser Defense Battery
createUnitScript("armllt")

__this:piece( "base", "flare", "turret", "sleeve", "barrel" )

__this.SIG_AIM	        = 2
__this.SMOKEPIECE1     = __this.base

#include "smokeunit.lh"
#include "EXPtype.lh"

__this.Create = function(this)
	this:hide( this.flare )
	this:dont_cache( this.flare )
	this:dont_cache( this.turret )
	this:dont_cache( this.sleeve )
	this:dont_cache( this.barrel )
	this:dont_shade( this.flare )
	this:dont_shade( this.turret )
	this:dont_shade( this.sleeve )
	this:dont_shade( this.barrel )
	this:start_script( this.SmokeUnit, this )
end

__this.AimPrimary = function(this, heading, pitch)
    this:set_script_value("AimPrimary", false)
	this:signal( this.SIG_AIM )
	set_signal_mask( this.SIG_AIM )

	heading = heading * TA2DEG
	pitch = pitch * TA2DEG

	this:turn( this.turret, y_axis, heading, 300 )
	this:turn( this.sleeve, x_axis, -pitch, 200 )
	this:wait_for_turn( this.turret, y_axis )
	this:wait_for_turn( this.sleeve, x_axis )
    this:set_script_value("AimPrimary", true)
end

__this.FirePrimary = function(this)
	this:show( this.flare )
	sleep( 0.15 )
	this:hide( this.flare )
end

__this.AimFromPrimary = function(this)
	return this.turret
end

__this.QueryPrimary = function(this)
	return this.flare
end

__this.SweetSpot = function(this)
	return this.base
end

__this.Killed = function(this, severity )
	hide( this.flare )
	if severity <= 25 then
		this:explode( this.barrel, BITMAPONLY + BITMAP1 )
		this:explode( this.base, BITMAPONLY + BITMAP2 )
		this:explode( this.flare, BITMAPONLY + BITMAP3 )
		this:explode( this.sleeve, BITMAPONLY + BITMAP4 )
		this:explode( this.turret, BITMAPONLY + BITMAP5 )
		return 1
	end

	if severity <= 50 then
		this:explode( this.barrel, FALL + BITMAP1 )
		this:explode( this.base, BITMAPONLY + BITMAP2 )
		this:explode( this.flare, FALL + BITMAP3 )
		this:explode( this.sleeve, SHATTER + BITMAP4 )
		this:explode( this.turret, FALL + BITMAP5 )
		return 2
	end

	if severity <= 99 then
		this:explode( this.barrel, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
		this:explode( this.base, BITMAPONLY + BITMAP2 )
		this:explode( this.flare, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
		this:explode( this.sleeve, SHATTER + BITMAP4 )
		this:explode( this.turret, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP5 )
		return 3
	end

	this:explode( this.barrel, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
	this:explode( this.base, BITMAPONLY + BITMAP2 )
	this:explode( this.flare, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
	this:explode( this.sleeve, SHATTER + EXPLODE_ON_HIT + BITMAP4 )
	this:explode( this.turret, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP5 )
	return 3
end

