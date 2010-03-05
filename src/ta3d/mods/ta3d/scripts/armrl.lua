-- Arm Rocket Tower
createUnitScript("armrl")

__this:piece( "base", "flare", "turret", "barrel", "launcher" )

__this.SMOKEPIECE1 = __this.base

#include "smokeunit.lh"
#include "EXPtype.lh"

__this.Create = function(this)
	this:hide( this.flare )
	this:dont_cache( this.barrel )
	this:dont_cache( this.launcher )
	this:dont_cache( this.turret )
	this:dont_cache( this.flare )
	this:dont_shade( this.barrel )
	this:dont_shade( this.launcher )
	this:dont_shade( this.turret )
	this:dont_shade( this.flare )
	this.next_barrel1 = 1
	this.aiming = false
	this:start_script( this.SmokeUnit, this )
end

__this.Save = function(this)
	return tostring(this.next_barrel1)
end

__this.Restore = function(this, data)
	this.next_barrel1 = tonumber(data)
end

__this.AimPrimary = function(this, heading, pitch)
	this:set_script_value("AimPrimary", false)
	if this.next_barrel1 == 1 then
		this:turn( this.barrel, z_axis, 0, 400 )
	elseif this.next_barrel1 == 2 then
		this:turn( this.barrel, z_axis, 120, 400 )
	else
		this:turn( this.barrel, z_axis, -120, 400 )
	end
	
	heading = heading * TA2DEG
	pitch = pitch * TA2DEG

	this:turn( this.turret, y_axis, heading, 250 )
	this:turn( this.launcher, x_axis, -pitch, 250 )

	if this.aiming then
		return
	end
	this.aiming = true
	
	while this:is_turning( this.barrel, z_axis ) or this:is_turning(this.turret, y_axis) or this:is_turning( this.launcher, x_axis) do
		this.yield()
	end
	this:set_script_value("AimPrimary", true)

	this.aiming = false
end

__this.FirePrimary = function(this)
	this.next_barrel1 = this.next_barrel1 + 1
	if this.next_barrel1 == 4 then
		this.next_barrel1 = 1
	end
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

__this.Killed = function( this, severity )
	if severity <= 25 then
		this:explode( this.barrel, BITMAPONLY + BITMAP1 )
		this:explode( this.base, BITMAPONLY + BITMAP2 )
		this:explode( this.flare, BITMAPONLY + BITMAP3 )
		this:explode( this.launcher, BITMAPONLY + BITMAP4 )
		this:explode( this.turret, BITMAPONLY + BITMAP5 )
		return 1
	end

	if severity <= 50 then
		this:explode( this.barrel, FALL + BITMAP1 )
		this:explode( this.base, BITMAPONLY + BITMAP2 )
		this:explode( this.flare, FALL + BITMAP3 )
		this:explode( this.launcher, SHATTER + BITMAP4 )
		this:explode( this.turret, FALL + BITMAP5 )
		return 2
	end

	if severity <= 99 then
		this:explode( this.barrel, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
		this:explode( this.base, BITMAPONLY + BITMAP2 )
		this:explode( this.flare, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
		this:explode( this.launcher, SHATTER + BITMAP4 )
		this:explode( this.turret, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP5 )
		return 3
	end

	this:explode( this.barrel, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
	this:explode( this.base, BITMAPONLY + BITMAP2 )
	this:explode( this.flare, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
	this:explode( this.launcher, SHATTER + EXPLODE_ON_HIT + BITMAP4 )
	this:explode( this.turret, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP5 )
	return 3
end

