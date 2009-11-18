-- Arm Mobile SAM Launcher

createUnitScript("armsam")
__this:piece( "flare1", "flare2", "base", "launcher", "turret" )

__this.SMOKEPIECE1 = __this.base
#include "smokeunit.lh"
#include "exptype.lh"
#include "hitweap.lh"

__this.Create = function(this)
	this:hide( this.flare1 )
	this:hide( this.flare2 )
	this.fire = false
	this.restore_delay = 3000
	this.aiming = false
	this:start_script( this.SmokeUnit, this )
end

__this.SetMaxReloadTime = function(this, time)
	this.restore_delay = time * 2
end

__this.RestoreAfterDelay = function(this)
	this:sleep( this.restore_delay )
	this:turn( this.turret, y_axis, 0, 90 )
	this:turn( this.launcher, x_axis, 0, 50 )
end

__this.AimPrimary = function(this, heading, pitch)
    this:set_script_value("AimPrimary", false)

	heading = heading * TA2DEG
	pitch = pitch * TA2DEG

	this:turn( this.turret, y_axis, heading, 250 )
	this:turn( this.launcher, x_axis, -pitch, 150 )
	if this.aiming then
		return
	end

	this.aiming = true
	while this:is_turning( this.turret, y_axis ) or this:is_turning( this.launcher, x_axis ) do
		this.yield()
	end

    this:set_script_value("AimPrimary", true)
	this.aiming = false

	this:start_script( this.RestoreAfterDelay, this)
end

__this.FirePrimary = function(this)
	if not this.fire then
		this:show( this.flare1 )
		this:sleep( 0.15 )
		this:hide( this.flare1 )
	else
		this:show( this.flare2 )
		this:sleep( 0.15 )
		this:hide( this.flare2 )
	end
	this.fire = not this.fire
end

__this.AimFromPrimary = function(this)
	return this.launcher
end

__this.QueryPrimary = function(this)
	return this.fire
end

__this.SweetSpot = function(this)
	return this.base
end

__this.Killed = function(this, severity)
	this:hide( this.flare1 )
	this:hide( this.flare2 )
	if severity <= 25 then
		this:explode( this.base, BITMAPONLY + BITMAP1 )
		this:explode( this.flare1, BITMAPONLY + BITMAP2 )
		this:explode( this.flare2, BITMAPONLY + BITMAP3 )
		this:explode( this.launcher, BITMAPONLY + BITMAP4 )
		this:explode( this.turret, BITMAPONLY + BITMAP5 )
		return 1
	end

	if severity <= 50 then
		this:explode( this.base, BITMAPONLY + BITMAP1 )
		this:explode( this.flare1, FALL + BITMAP )
		this:explode( this.flare2, FALL + BITMAP3 )
		this:explode( this.launcher, SHATTER + BITMAP4 )
		this:explode( this.turret, FALL + BITMAP5 )
		return 2
	end

	if severity <= 99 then
		this:explode( this.base, BITMAPONLY + BITMAP1 )
		this:explode( this.flare1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
		this:explode( this.flare2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
		this:explode( this.launcher, SHATTER + BITMAP4 )
		this:explode( this.turret, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP5 )
		return 3
	end

	this:explode( this.base, BITMAPONLY + BITMAP1 )
	this:explode( this.flare1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
	this:explode( this.flare2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
	this:explode( this.launcher, SHATTER + EXPLODE_ON_HIT + BITMAP4 )
	this:explode( this.turret, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP5 )
	return 3
end
