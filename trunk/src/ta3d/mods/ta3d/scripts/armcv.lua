-- Arm Construction Vehicle

createUnitScript("armcv")

__this:piece( "base", "beam", "arm", "door1", "door2", "nano", "plate", "turret" )

__this.buildheading = 0

__this.SMOKEPIECE1     = __this.base
__this.ANIM_VARIABLE   = true

#include "StateChg.lh"
#include "smokeunit.lh"
#include "exptype.lh"

__this.activatescr = function(this)
    this:turn( this.door1, z_axis, -90, 90 )
    this:turn( this.door2, z_axis, 90, 90 )
    
    this:wait_for_turn( this.door1, z_axis )
    this:wait_for_turn( this.door2, z_axis )
    
    this:turn( this.arm, x_axis, 90, 90 )
    this:turn( this.nano, x_axis, -90, 90 )

    this:wait_for_turn( this.arm, x_axis )
    this:wait_for_turn( this.nano, x_axis )
end

__this.deactivatescr = function(this)
    this:turn( this.arm, x_axis, 0, 90 )
    this:turn( this.nano, x_axis, 0, 90 )

    this:wait_for_turn( this.arm, x_axis )
    this:wait_for_turn( this.nano, x_axis )

    this:turn( this.door1, z_axis, 0, 90 )
    this:turn( this.door2, z_axis, 0, 90 )
    
    this:wait_for_turn( this.door1, z_axis )
    this:wait_for_turn( this.door2, z_axis )
end

__this.Go = function(this)
	this:activatescr()
	this:turn( this.turret, y_axis, this.buildheading, 160 )
	this:wait_for_turn( this.turret, y_axis )
	this:set( INBUILDSTANCE, true )
end

__this.Stop = function(this)
	this:set( INBUILDSTANCE, false )
	this:turn( this.turret, y_axis, 0, 160 )
	this:wait_for_turn( this.turret, y_axis )
	this:deactivatescr()
end

__this.ACTIVATECMD     = __this.Go
__this.DEACTIVATECMD   = __this.Stop

__this.Create = function(this)
	this.buildheading = 0
	this:InitState()
	this:start_script( this.SmokeUnit, this )
end

__this.Activate = function(this)
	this:start_script( this.RequestState, this, ACTIVE )
end

__this.Deactivate = function(this)
	this:start_script( this.RequestState, this, INACTIVE )
end

__this.StartBuilding = function(this, heading)
	this.buildheading = heading * TA2DEG
	this:start_script( this.RequestState, this, ACTIVE )
end

__this.StopBuilding = function(this)
	this:start_script( this.RequestState, this, INACTIVE )
end

__this.QueryNanoPiece = function(this)
	return this.beam
end

__this.TargetHeading = function(this, heading)
	this.buildheading = -heading * TA2DEG
end

__this.SweetSpot = function(this)
	return this.base
end

__this.Killed = function(this, severity)
	if severity <= 25 then
		this:explode( this.arm, BITMAPONLY + BITMAP1 )
		this:explode( this.base, BITMAPONLY + BITMAP2 )
		this:explode( this.beam, BITMAPONLY + BITMAP3 )
		this:explode( this.door1, BITMAPONLY + BITMAP4 )
		this:explode( this.door2, BITMAPONLY + BITMAP5 )
		this:explode( this.nano, BITMAPONLY + BITMAP1 )
		this:explode( this.plate, BITMAPONLY + BITMAP2 )
		this:explode( this.turret, BITMAPONLY + BITMAP3 )
		return 1
	end

	if severity <= 50 then
		this:explode( this.arm, FALL + BITMAP1 )
		this:explode( this.base, BITMAPONLY + BITMAP2 )
		this:explode( this.beam, FALL + BITMAP3 )
		this:explode( this.door1, BITMAPONLY + BITMAP4 )
		this:explode( this.door2, BITMAPONLY + BITMAP5 )
		this:explode( this.nano, SHATTER + BITMAP1 )
		this:explode( this.plate, BITMAPONLY + BITMAP2 )
		this:explode( this.turret, FALL + BITMAP3 )
		return 2
	end

	if severity <= 99 then
		this:explode( this.arm, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
		this:explode( this.base, BITMAPONLY + BITMAP2 )
		this:explode( this.beam, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
		this:explode( this.door1, BITMAPONLY + BITMAP4 )
		this:explode( this.door2, BITMAPONLY + BITMAP5 )
		this:explode( this.nano, SHATTER + BITMAP1 )
		this:explode( this.plate, BITMAPONLY + BITMAP2 )
		this:explode( this.turret, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
		return 3
	end

	this:explode( this.arm, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
	this:explode( this.base, BITMAPONLY + BITMAP2 )
	this:explode( this.beam, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
	this:explode( this.door1, BITMAPONLY + BITMAP4 )
	this:explode( this.door2, BITMAPONLY + BITMAP5 )
	this:explode( this.nano, SHATTER + EXPLODE_ON_HIT + BITMAP1 )
	this:explode( this.plate, BITMAPONLY + BITMAP2 )
	this:explode( this.turret, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
	return 3
end

