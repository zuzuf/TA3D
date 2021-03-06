-- Arm Plasma Defense Battery

createUnitScript("armguard")

__this:piece( "flare1", "flare2", "base", "turret", "sleeves", "barrel1", "barrel2" )

__this.fire = 0

__this.SMOKEPIECE1 = __this.base

#include "exptype.lh"
#include "smokeunit.lh"

__this.Create = function(this)
	this:hide( this.flare1 )
	this:hide( this.flare2 )
	this:dont_cache( this.flare1 )
	this:dont_cache( this.flare2 )
	this:dont_cache( this.barrel1 )
	this:dont_cache( this.barrel2 )
	this:dont_cache( this.sleeves )
	this:dont_cache( this.turret )
	this.fire = 0
	this.aiming = false
	this:start_script( this.SmokeUnit, this )
end

__this.Save = function(this)
	return tostring(this.fire)
end

__this.Restore = function(this, data)
	this.fire = tonumber(data)
end

__this.AimPrimary = function(this, heading, pitch)

	this:set_script_value("AimPrimary", false)

	heading = heading * TA2DEG
	pitch = pitch * TA2DEG

	this:turn( this.turret, y_axis, heading, 30 )
	this:turn( this.sleeves, x_axis, -pitch, 45 )

	if this.aiming then			-- already aiming
		return
	end
	
	this.aiming = true
	while this:is_turning( this.turret, y_axis ) or this:is_turning( this.sleeves, x_axis ) do
		this.yield()
	end
	this:set_script_value("AimPrimary", true)
	this.aiming = false
end

__this.FirePrimary = function(this)
	if this.fire == 0 then
		this:move_piece_now( this.barrel1, z_axis, -2.5 )
		this:show( this.flare1 )
		this:sleep( 0.15 )
		this:hide( this.flare1 )
		this:move( this.barrel1, z_axis, 0, 1 )
		this.fire = 1
	else
		this:move_piece_now( this.barrel2, z_axis, -2.5 )
		this:show( this.flare2 )
		this:sleep( 0.15 )
		this:hide( this.flare2 )
		this:move( this.barrel2, z_axis, 0, 1 )
		this.fire = 0
	end
end

__this.QueryPrimary = function(this)
    if this.fire == 0 then
    	return this.flare1
    end
    return this.flare2
end

__this.AimFromPrimary = function(this)
	return this.turret
end

__this.SweetSpot = function(this)
	return this.base
end

__this.Killed = function( this, severity )
	this:hide( this.flare1 )
	this:hide( this.flare2 )
	if severity <= 25 then
		this:explode( this.barrel1, BITMAPONLY + BITMAP1 )
		this:explode( this.barrel2, BITMAPONLY + BITMAP2 )
		this:explode( this.base, BITMAPONLY + BITMAP3 )
		this:explode( this.flare1, BITMAPONLY + BITMAP4 )
		this:explode( this.flare1, BITMAPONLY + BITMAP5 )
		this:explode( this.sleeves, BITMAPONLY + BITMAP1 )
		this:explode( this.turret, BITMAPONLY + BITMAP2 )
		return 1
	end

	if severity <= 50 then
		this:explode( this.barrel1, FALL + BITMAP1 )
		this:explode( this.barrel2, FALL + BITMAP2 )
		this:explode( this.base, BITMAPONLY + BITMAP3 )
		this:explode( this.flare1, FALL + BITMAP4 )
		this:explode( this.flare1, FALL + BITMAP5 )
		this:explode( this.sleeves, SHATTER + BITMAP1 )
		this:explode( this.turret, BITMAPONLY + BITMAP2 )
		return 2
	end

	if severity <= 99 then
		this:explode( this.barrel1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
		this:explode( this.barrel2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
		this:explode( this.base, BITMAPONLY + BITMAP3 )
		this:explode( this.flare1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4 )
		this:explode( this.flare1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP5 )
		this:explode( this.sleeves, SHATTER + BITMAP1 )
		this:explode( this.turret, BITMAPONLY + BITMAP2 )
		return 3
	end

	this:explode( this.barrel1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
	this:explode( this.barrel2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
	this:explode( this.base, BITMAPONLY + BITMAP3 )
	this:explode( this.flare1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4 )
	this:explode( this.flare1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP5 )
	this:explode( this.sleeves, SHATTER + EXPLODE_ON_HIT + BITMAP1 )
	this:explode( this.turret, BITMAPONLY + BITMAP2 )
	return 3
end

