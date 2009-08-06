-- Template script
-- this file contains the basic function prototypes you have to
-- write in order to have a working unit
-- NB: some may not be used depending on your unit

createUnitScript("armtpl")

__this:piece( "piece1", "piece2", "piece3" )

__this.SMOKEPIECE1 = __this.piece1

#include "smokeunit.lh"		-- almost all units should produce smoke when damaged
#include "exptype.lh"

__this.Create = function(this)	-- this is the first function called by the unit
	this:start_script( this.SmokeUnit, this )
end

__this.Save = function(this)
	return ""
end

__this.Restore = function(this, data)	-- this function is called after a game has been loaded to allow
										-- restoring your unit data
end

__this.AimPrimary = function(this, heading, pitch)
	this:set_script_value("AimPrimary", false)

	-- your aiming code goes here

	this:set_script_value("AimPrimary", true)
end

__this.FirePrimary = function(this)
	-- here you fire your primary weapon
end

__this.QueryPrimary = function(this)
    return this.piece2
end

__this.AimFromPrimary = function(this)
	return this.piece3
end

__this.SweetSpot = function(this)
	return this.piece1
end

__this.Killed = function( this, severity )		-- the last function called in all the unit's life
	if severity <= 25 then
		this:explode( this.piece1, BITMAPONLY + BITMAP1 )
		this:explode( this.piece2, BITMAPONLY + BITMAP2 )
		this:explode( this.piece3, BITMAPONLY + BITMAP3 )
		return 1
	end

	if severity <= 50 then
		this:explode( this.piece1, FALL + BITMAP1 )
		this:explode( this.piece2, FALL + BITMAP2 )
		this:explode( this.piece3, BITMAPONLY + BITMAP3 )
		return 2
	end

	if severity <= 99 then
		this:explode( this.piece1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
		this:explode( this.piece2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
		this:explode( this.piece3, BITMAPONLY + BITMAP3 )
		return 3
	end

	this:explode( this.piece1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
	this:explode( this.piece2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
	this:explode( this.piece3, BITMAPONLY + BITMAP3 )
	return 3
end

