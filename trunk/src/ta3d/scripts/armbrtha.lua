-- Arm Big Bertha Cannon

createUnitScript("armbrtha")

__this:piece( "base", "flare", "turret", "barrel", "sleeve" )

__this.SMOKEPIECE1 = __this.base

#include "smokeunit.lh"
#include "exptype.lh"

__this.Create = function(this)
	this:hide( this.flare )
	this:dont_cache( this.flare )
	this:dont_cache( this.barrel )
	this:dont_cache( this.sleeve )
	this:dont_cache( this.turret )
	this.aiming = false
	this:start_script( this.SmokeUnit, this )
end

__this.AimPrimary = function(this, heading, pitch)
    this:set_script_value("AimPrimary", false)

	heading = heading * TA2DEG
	pitch = pitch * TA2DEG

	this:turn( this.turret, y_axis, heading, 5 )
	this:turn( this.sleeve, x_axis, -pitch, 2 )
	if this.aiming then
		return
	end

	this.aiming = true
	while this:is_turning( this.turret, y_axis ) or this:is_turning( this.sleeve, x_axis ) do
		this.yield()
	end

    this:set_script_value("AimPrimary", true)
	this.aiming = false
end

__this.FirePrimary = function(this)
	this:move( this.barrel, z_axis, -5, 500 )
	this:show( this.flare )
	this:sleep( 0.25 )
	this:hide( this.flare )
	this:wait_for_move( this.barrel, z_axis )
	this:move( this.barrel, z_axis, 0, 3.0 )
end

__this.TargetCleared = function(this)
	this:stop_spin( this.turret, y_axis )
	this:stop_spin( this.sleeve, x_axis )
end

__this.QueryPrimary = function(this)
	return this.flare
end

__this.AimFromPrimary = function(this)
	return this.turret
end

__this.SweetSpot = function(this)
	return this.base
end

__this.Killed = function(this, severity)
	this:hide( this.flare )
    if severity <= 25 then
		this:explode( barrel, BITMAPONLY + BITMAP1 )
		this:explode( base, BITMAPONLY + BITMAP2 )
		this:explode( flare, BITMAPONLY + BITMAP3 )
		this:explode( sleeve, BITMAPONLY + BITMAP4 )
		this:explode( turret, BITMAPONLY + BITMAP5 )
		return 1
	end

	if severity <= 50 then
		this:explode( barrel, BITMAPONLY + BITMAP1 )
		this:explode( base, BITMAPONLY + BITMAP2 )
		this:explode( flare, FALL + BITMAP3 )
		this:explode( sleeve, SHATTER + BITMAP4 )
		this:explode( turret, FALL + BITMAP5 )
		return 2
	end

	if severity <= 99 then
		this:explode( barrel, BITMAPONLY + BITMAP1 )
		this:explode( base, BITMAPONLY + BITMAP )
		this:explode( flare, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
		this:explode( sleeve, SHATTER + BITMAP )
		this:explode( turret, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP5 )
		return 3
	end

	this:explode( this.barrel, BITMAPONLY + BITMAP1 )
	this:explode( this.base, BITMAPONLY + BITMAP2 )
	this:explode( this.flare, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
	this:explode( this.sleeve, SHATTER + EXPLODE_ON_HIT + BITMAP4 )
	this:explode( this.turret, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP5 )
	return 3
end
