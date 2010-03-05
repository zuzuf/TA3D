-- Arm Fusion plant script
createUnitScript("armfus")

__this:piece( "base" )

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
		this:explode( this.base, BITMAPONLY + BITMAP2 )
		return 1
	end

	if severity <= 50 then
		this:explode( this.base, BITMAPONLY + BITMAP2 )
		return 2
	end

	if severity <= 99 then
		this:explode( this.base, BITMAPONLY + BITMAP2 )
		return 3
	end

	this:explode( this.base, BITMAPONLY + BITMAP2 )
	return 3
end

