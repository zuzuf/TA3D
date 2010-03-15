-- Arm Vehicle Plant Script

createUnitScript("armvp")

__this:piece( "base", "pad", "beam1", "beam2", "doo2", "door1", "nano1", "nano2", "plate1", "plate2", "post1", "post2", "side1", "side2" )

__this.SIG_ACTIVATE = 2
__this.SMOKEPIECE1 = __this.base
__this.ANIM_VARIABLE = true

#include "exptype.lh"
#include "StateChg.lh"
#include "smokeunit.lh"
#include "yard.lh"

__this.activatescr = function (this)
    this:turn( this.side1, z_axis, -90, 60 )
    this:turn( this.side2, z_axis, 90, 60 )

    this:wait_for_turn( this.side1, z_axis )
    this:wait_for_turn( this.side2, z_axis )

    this:turn( this.door1, z_axis, -90, 60 )
    this:turn( this.doo2, z_axis, 90, 60 )

    this:wait_for_turn( this.door1, z_axis )
    this:wait_for_turn( this.doo2, z_axis )
    
    this:move( this.door1, x_axis, 10, 10 )
    this:move( this.doo2, x_axis, -10, 10 )

    this:move( this.plate1, x_axis, -8, 8 )
    this:move( this.plate2, x_axis, 7.5, 7.5 )

    this:wait_for_move( this.plate1, x_axis )
    this:wait_for_move( this.plate2, x_axis )
    this:wait_for_move( this.door1, x_axis )
    this:wait_for_move( this.doo2, x_axis )

    this:turn( this.nano1, z_axis, 30, 60 )
    this:turn( this.nano2, z_axis, -30, 60 )
    this:wait_for_turn( this.nano1, z_axis )
    this:wait_for_turn( this.nano2, z_axis )
end

__this.deactivatescr = function (this)
    this:turn( this.nano1, z_axis, 0, 60 )
    this:turn( this.nano2, z_axis, 0, 60 )
    this:wait_for_turn( this.nano1, z_axis )
    this:wait_for_turn( this.nano2, z_axis )

    this:move( this.door1, x_axis, 0, 10 )
    this:move( this.doo2, x_axis, 0, 10 )

    this:move( this.plate1, x_axis, 0, 8 )
    this:move( this.plate2, x_axis, 0, 7.5 )

    this:wait_for_move( this.plate1, x_axis )
    this:wait_for_move( this.plate2, x_axis )
    this:wait_for_move( this.door1, x_axis )
    this:wait_for_move( this.doo2, x_axis )

    this:turn( this.door1, z_axis, 0, 60 )
    this:turn( this.doo2, z_axis, 0, 60 )

    this:wait_for_turn( this.door1, z_axis )
    this:wait_for_turn( this.doo2, z_axis )

    this:turn( this.side1, z_axis, 0, 60 )
    this:turn( this.side2, z_axis, 0, 60 )

    this:wait_for_turn( this.side1, z_axis )
    this:wait_for_turn( this.side2, z_axis )
end

__this.Go = function (this)
	this:dont_cache( this.doo2 )
	this:dont_cache( this.door1 )
	this:dont_cache( this.nano1 )
	this:dont_cache( this.nano2 )
	this:dont_cache( this.pad )
	this:dont_cache( this.plate1 )
	this:dont_cache( this.plate2 )
	this:dont_cache( this.post1 )
	this:dont_cache( this.post2 )
	this:dont_cache( this.side1 )
	this:dont_cache( this.side2 )
	this:activatescr()
	this:OpenYard()
	this:set( INBUILDSTANCE, true )
end

__this.Stop = function(this)
	this:set( INBUILDSTANCE, false )
	this:CloseYard()
	this:deactivatescr()
	this:cache( this.doo2 )
	this:cache( this.door1 )
	this:cache( this.nano1 )
	this:cache( this.nano2 )
	this:cache( this.pad )
	this:cache( this.plate1 )
	this:cache( this.plate2 )
	this:cache( this.post1 )
	this:cache( this.post2 )
	this:cache( this.side1 )
	this:cache( this.side2 )
end

__this.ACTIVATECMD = __this.Go
__this.DEACTIVATECMD = __this.Stop

__this.Create = function(this)
	this.spray = 0
	this:InitState()
	this:start_script( this.SmokeUnit, this )
end

__this.QueryNanoPiece = function(this, piecenum)
	if this.spray == 0 then
		piecenum = this.beam1
	else
		piecenum = this.beam2
	end
	this.spray = (this.spray + 1) % 2
	return piecenum
end

__this.Activate = function(this)
	this:signal( this.SIG_ACTIVATE )
	this:start_script( this.RequestState, this, ACTIVE )
end

__this.Deactivate = function(this)
	this:signal( this.SIG_ACTIVATE )
	set_signal_mask( this.SIG_ACTIVATE )
	sleep( 5 )
	set_signal_mask( 0 )
	this:start_script( this.RequestState, this, INACTIVE )
end

__this.StartBuilding = function(this)
	this:spin( this.pad, y_axis, 30 )
end

__this.StopBuilding = function(this)
	this:stop_spin( this.pad, y_axis )
end

__this.QueryBuildInfo = function(this, piecenum)
	return this.pad
end

__this.SweetSpot = function (this,piecenum)
	return this.base
end

__this.Killed = function(this, severity )
	if severity <= 25 then
		this:explode( this.base,	BITMAPONLY + BITMAP1 )
		this:explode( this.doo2,	BITMAPONLY + BITMAP4 )
		this:explode( this.door1,	BITMAPONLY + BITMAP5 )
		this:explode( this.nano1,	BITMAPONLY + BITMAP1 )
		this:explode( this.nano2,	BITMAPONLY + BITMAP2 )
		this:explode( this.pad,	BITMAPONLY + BITMAP3 )
		this:explode( this.plate1,	BITMAPONLY + BITMAP4 )
		this:explode( this.plate2,	BITMAPONLY + BITMAP5 )
		this:explode( this.post1,	BITMAPONLY + BITMAP1 )
		this:explode( this.post2,	BITMAPONLY + BITMAP2 )
		this:explode( this.side1,	BITMAPONLY + BITMAP3 )
		this:explode( this.side2,	BITMAPONLY + BITMAP4 )
		return 1
	end

	if severity <= 50 then
		this:explode( this.base,	BITMAPONLY + BITMAP1 )
		this:explode( this.doo2,	FALL + BITMAP4 )
		this:explode( this.door1,	BITMAPONLY + BITMAP5 )
		this:explode( this.nano1,	BITMAPONLY + BITMAP1 )
		this:explode( this.nano2,	FALL + BITMAP2 )
		this:explode( this.pad,	BITMAPONLY + BITMAP3 )
		this:explode( this.plate1,	SHATTER + BITMAP4 )
		this:explode( this.plate2,	BITMAPONLY + BITMAP5 )
		this:explode( this.post1,	FALL + BITMAP1 )
		this:explode( this.post2,	FALL + BITMAP2 )
		this:explode( this.side1,	BITMAPONLY + BITMAP3 )
		this:explode( this.side2,	BITMAPONLY + BITMAP4 )
		return 2
	end

	if severity <= 99 then
		this:explode( this.base,	BITMAPONLY + BITMAP1 )
		this:explode( this.doo2,	FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4 )
		this:explode( this.door1,	BITMAPONLY + BITMAP5 )
		this:explode( this.nano1,	BITMAPONLY + BITMAP1 )
		this:explode( this.nano2,	FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
		this:explode( this.pad,	BITMAPONLY + BITMAP3 )
		this:explode( this.plate1,	SHATTER + BITMAP4 )
		this:explode( this.plate2,	SHATTER + BITMAP5 )
		this:explode( this.post1,	FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
		this:explode( this.post2,	FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
		this:explode( this.side1,	BITMAPONLY + BITMAP3 )
		this:explode( this.side2,	BITMAPONLY + BITMAP4 )
		return 3
	end

	this:explode( this.base,	BITMAPONLY + BITMAP1 )
	this:explode( this.doo2,	FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4 )
	this:explode( this.door1,	BITMAPONLY + BITMAP5 )
	this:explode( this.nano1,	BITMAPONLY + BITMAP1 )
	this:explode( this.nano2,	FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
	this:explode( this.pad,	BITMAPONLY + BITMAP3 )
	this:explode( this.plate1,    SHATTER + EXPLODE_ON_HIT + BITMAP4 )
	this:explode( this.plate2,	SHATTER + EXPLODE_ON_HIT + BITMAP5 )
	this:explode( this.post1,	FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
	this:explode( this.post2,	FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
	this:explode( this.side1,	BITMAPONLY + BITMAP3 )
	this:explode( this.side2,	BITMAPONLY + BITMAP4 )
	return 3
end

