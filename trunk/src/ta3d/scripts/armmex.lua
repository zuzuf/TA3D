-- Arm Metal Extractor Script
createUnitScript("armmex")

__this:piece( "base", "arms" )

__this.SMOKEPIECE1 = __this.base

#include "StateChg.lh"
#include "smokeunit.lh"
#include "exptype.lh"

__this.spinspeed = 0
__this.spinacc = 0.5
__this.spindec = 1.0

__this.Go = function(this)
	this:spin( this.arms, y_axis, this.spinspeed, this.spinacc )
end

__this.Stop = function(this)
	this:stop_spin( this.arms, y_axis, this.spindec )
	this:wait_for_turn( this.arms, y_axis )
end

__this.ACTIVATECMD = __this.Go
__this.DEACTIVATECMD = __this.Stop

__this.Create = function(this)
	this.spinspeed = 0
	this.spinacc = 0.5
	this.spindec = 1.0
	this:dont_shade( this.arms )
	this:dont_cache( this.arms )
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

__this.SetSpeed = function(this, the_speed)
	this.spinspeed = the_speed * 0.25
end

__this.Killed = function(this, severity )
	if severity <= 25 then
		this:explode( this.arms, BITMAPONLY + BITMAP1 )
		this:explode( this.base, BITMAPONLY + BITMAP2 )
		return 1
	end

	if severity <= 50 then
		this:explode( this.arms, SHATTER + BITMAP1 )
		this:explode( this.base, BITMAPONLY + BITMAP2 )
		return 2
	end

	if severity <= 99 then
		this:explode( this.arms, SHATTER + EXPLODE_ON_HIT + BITMAP1 )
		this:explode( this.base, BITMAPONLY + BITMAP2 )
		return 3
	end

	this:explode( this.arms, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
	this:explode( this.base, SHATTER + EXPLODE_ON_HIT + BITMAP2 )
	return 3
end
