-- Arm Geo Power Plant Script
createUnitScript("armgeo")

__this:piece("base", "smoke", "pieces")

__this.SMOKEPIECE1 = __this.base
#include "smokeunit.lh"
#include "EXPtype.lh"

__this.Create = function(this)
	this:start_script( this.SmokeUnit, this )
	end

__this.SweetSpot = function(this)
	return this.base
	end

__this.Killed = function( this, severity )
	if severity <= 25 then
		this:explode( this.base, BITMAPONLY + BITMAP1 )
		this:explode( this.smoke, BITMAPONLY + BITMAP2 )
		this:explode( this.pieces, BITMAPONLY + BITMAP2 )
		return 1
	end

	if severity <= 50 then
		this:explode( this.base, BITMAPONLY + BITMAP1 )
		this:explode( this.smoke, FALL + BITMAP2 )
		this:explode( this.pieces, BITMAPONLY + BITMAP2 )
		return 2
	end

	if severity <= 99 then
		this:explode( this.base, BITMAPONLY + BITMAP1 )
		this:explode( this.smoke, FALL + BITMAP2 )
		this:explode( this.pieces, SHATTER + BITMAP2 )
		return 3
	end

	this:explode( this.base, BITMAPONLY + BITMAP1 )
	this:explode( this.smoke, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
	this:explode( this.pieces, SHATTER + EXPLODE_ON_HIT + BITMAP2 )
	return 3
end

