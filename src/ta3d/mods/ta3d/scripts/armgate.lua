-- Arm Galactic Gate Script

createUnitScript("armgate")
__this:piece( "base", "arm", "dish", "door", "flap1", "flap2", "flap3", "gun", "plate", "turret" )

__this.SMOKEPIECE1 = __this.base
#include "smokeunit.lh"
#include "exptype.lh"

__this.Create = function(this)
	this:dont_shade( this.flap1 )
	this:dont_shade( this.flap2 )
	this:dont_shade( this.flap3 )
	this:dont_shade( this.dish )
	this:dont_shade( this.door )
	this:dont_shade( this.arm )
	this:dont_shade( this.gun )
	this:dont_shade( this.plate )
	this:dont_shade( this.turret )
	this:start_script( this.SmokeUnit, this )
end

__this.SweetSpot = function(this)
	return this.base
end

__this.Killed = function(this, severity)
	if severity <= 25 then
		this:explode( this.arm, BITMAPONLY + BITMAP1 )
		this:explode( this.base, BITMAPONLY + BITMAP2 )
		this:explode( this.dish, BITMAPONLY + BITMAP3 )
		this:explode( this.door, BITMAPONLY + BITMAP4 )
		this:explode( this.flap1, BITMAPONLY + BITMAP5 )
		this:explode( this.flap2, BITMAPONLY + BITMAP1 )
		this:explode( this.flap3, BITMAPONLY + BITMAP2 )
		this:explode( this.gun, BITMAPONLY + BITMAP3 )
		this:explode( this.plate, BITMAPONLY + BITMAP4 )
		this:explode( this.turret, BITMAPONLY + BITMAP5 )
		return 1
	end

	if severity <= 50 then
		this:explode( this.arm, FALL + BITMAP1 )
		this:explode( this.base, BITMAPONLY + BITMAP2 )
		this:explode( this.dish, BITMAPONLY + BITMAP3 )
		this:explode( this.door, FALL + BITMAP4 )
		this:explode( this.flap1, FALL + BITMAP5 )
		this:explode( this.flap2, FALL + BITMAP1 )
		this:explode( this.flap3, FALL + BITMAP2 )
		this:explode( this.gun, FALL + BITMAP3 )
		this:explode( this.plate, FALL + BITMAP4 )
		this:explode( this.turret, SHATTER + BITMAP5 )
		return 2
	end

	if severity <= 99 then
		this:explode( this.arm, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
		this:explode( this.base, BITMAPONLY + BITMAP2 )
		this:explode( this.dish, BITMAPONLY + BITMAP3 )
		this:explode( this.door, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4 )
		this:explode( this.flap1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP5 )
		this:explode( this.flap2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
		this:explode( this.flap3, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
		this:explode( this.gun, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
		this:explode( this.plate, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4 )
		this:explode( this.turret, SHATTER + BITMAP5 )
		return 3
	end

	this:explode( this.arm, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
	this:explode( this.base, BITMAPONLY + BITMAP2 )
	this:explode( this.dish, BITMAPONLY + BITMAP3 )
	this:explode( this.door, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4 )
	this:explode( this.flap1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP5 )
	this:explode( this.flap2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
	this:explode( this.flap3, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
	this:explode( this.gun, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
	this:explode( this.plate, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4 )
	this:explode( this.turret, SHATTER + EXPLODE_ON_HIT + BITMAP5 )
	return 3
end
