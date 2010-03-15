-- Arm Radar Tower script
createUnitScript("armrad")

__this:piece("base","dish","ear1","ear2")

__this.SMOKEPIECE1 = __this.base

#include "exptype.lh"
#include "StateChg.lh"
#include "smokeunit.lh"

__this.Go = function (this)
	this:spin( this.dish, y_axis, 60 )
	this:spin( this.ear1, x_axis, 120 )
	this:spin( this.ear2, x_axis, -120 )
end

__this.Stop = function(this)
	this:spin( this.dish, y_axis, 0 )
	this:spin( this.ear1, x_axis, 0 )
	this:spin( this.ear2, x_axis, 0 )
end

__this.ACTIVATECMD = __this.Go
__this.DEACTIVATECMD = __this.Stop

__this.Create = function(this)
	this:dont_cache( this.dish )
	this:dont_cache( this.ear1 )
	this:dont_cache( this.ear2 )
	this:dont_shade( this.dish )
	this:dont_shade( this.ear1 )
	this:dont_shade( this.ear2 )
	this:InitState()
	this:start_script( this.SmokeUnit, this )
end

__this.Activate = function(this)
	this:start_script( this.RequestState, this, ACTIVE )
end

__this.Deactivate = function(this)
	this:start_script( this.RequestState, this, INACTIVE )
end

__this.SweetSpot = function(this)
	return this.base
end

__this.Killed = function( this, severity )
	if severity <= 25 then
		this:explode( this.base, BITMAPONLY + BITMAP1 )
		this:explode( this.dish, BITMAPONLY + BITMAP2 )
		this:explode( this.ear1, BITMAPONLY + BITMAP3 )
		this:explode( this.ear2, BITMAPONLY + BITMAP4 )
		return 1
	end

	if severity <= 50 then
		this:explode( this.base, BITMAPONLY + BITMAP1 )
		this:explode( this.dish, SHATTER + BITMAP2 )
		this:explode( this.ear1, FALL + BITMAP3 )
		this:explode( this.ear2, FALL + BITMAP4 )
		return 2
	end

	if severity <= 99 then
		this:explode( this.base, BITMAPONLY + BITMAP1 )
		this:explode( this.dish, SHATTER + BITMAP2 )
		this:explode( this.ear1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
		this:explode( this.ear2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4 )
		return 3
	end

	this:explode( this.base, BITMAPONLY + BITMAP1 )
	this:explode( this.dish, SHATTER + EXPLODE_ON_HIT + BITMAP2 )
	this:explode( this.ear1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
	this:explode( this.ear2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4 )
	return 3
end
