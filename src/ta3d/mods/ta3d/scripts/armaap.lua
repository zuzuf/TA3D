-- Arm Advanced Aircraft Plant

createUnitScript("armaap")

__this:piece( "base", "pad", "beam1", "beam2", "building1", "building2", "nano1", "nano2", "nanobox1", "nanobox2", "radar", "lights" )

__this.SIG_ACTIVATE = 2
__this.SMOKEPIECE1 = __this.building1
__this.SMOKEPIECE2 = __this.building2
__this.SMOKEPIECE3 = __this.pad
__this.ANIM_VARIABLE = true

#include "StateChg.lh"
#include "smokeunit.lh"
#include "exptype.lh"
#include "yard.lh"

__this.activatescr = function(this)
	this:move( this.building1, x_axis, 10, 50)
	this:move( this.building2, x_axis, -10, 50)
	this:sleep(0.2)
	this:move( this.nanobox1, x_axis, 10, 50)
	this:move( this.nanobox2, x_axis, -10, 50)
	this:sleep(0.2)
	this:move( this.nano1, z_axis, 4.5, 10)
	this:move( this.nano2, z_axis, 4.5, 10)
end

__this.deactivatescr = function(this)
	this:move( this.nano1, z_axis, 0, 10)
	this:move( this.nano2, z_axis, 0, 10)
	this:sleep(0.2)
	this:move( this.nanobox1, x_axis, 0, 50)
	this:move( this.nanobox2, x_axis, 0, 50)
	this:sleep(0.2)
	this:move( this.building1, x_axis, 0, 50)
	this:move( this.building2, x_axis, 0, 50)
end

__this.Go = function(this)
	this:dont_cache( this.beam1 )
	this:dont_cache( this.beam2 )
	this:dont_cache( this.building1 )
	this:dont_cache( this.building2 )
	this:dont_cache( this.nano1 )
	this:dont_cache( this.nano2 )
	this:dont_cache( this.nanobox1 )
	this:dont_cache( this.nanobox2 )
	this:dont_cache( this.pad )
	this:activatescr()
	this:OpenYard()
	this:set( INBUILDSTANCE, true )
end

__this.Stop = function(this)
	this:set( INBUILDSTANCE, false )
	this:CloseYard()
	this:deactivatescr()
	this:cache( this.beam1 )
	this:cache( this.beam2 )
	this:cache( this.building1 )
	this:cache( this.building2 )
	this:cache( this.nano1 )
	this:cache( this.nano2 )
	this:cache( this.nanobox1 )
	this:cache( this.nanobox2 )
	this:cache( this.pad )
end

__this.ACTIVATECMD = __this.Go
__this.DEACTIVATECMD = __this.Stop

__this.Create = function(this)
	this:dont_cache( this.lights )
	this:dont_shade( this.lights )
	this:dont_cache( this.radar )
	this:dont_shade( this.radar )
	this:dont_shade( this.beam1 )
	this:dont_shade( this.beam2 )
	this:dont_shade( this.building1 )
	this:dont_shade( this.building2 )
	this:dont_shade( this.nano1 )
	this:dont_shade( this.nano2 )
	this:dont_shade( this.nanobox1 )
	this:dont_shade( this.nanobox2 )
	this:dont_shade( this.pad )
	this.spray = false
	this:InitState()
	this:start_script( this.SmokeUnit, this )
	while this:get(BUILD_PERCENT_LEFT) > 0.0 do
		this:sleep( 1.0 )
	end
	this:spin( this.radar, y_axis, 150 )
end

__this.QueryNanoPiece = function(this, piecenum)
	if this.spray then
		piecenum = this.beam2
	else
		piecenum = this.beam1
	end
	this.spray = not this.spray
	return piecenum
end

__this.Activate = function(this)
	this:signal( this.SIG_ACTIVATE )
	this:start_script( this.RequestState, this, ACTIVE )
end

__this.Deactivate = function(this)
	this:signal( SIG_ACTIVATE )
	this:set_signal_mask( SIG_ACTIVATE )
	this:sleep( 5.0 )
	this:set_signal_mask( 0 )
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

__this.SweetSpot = function(this, piecenum)
	return this.base
end

__this.Killed = function(this, severity)
	if severity <= 25 then
		this:explode( this.base, BITMAPONLY + BITMAP1 )
		this:explode( this.building1, BITMAPONLY + BITMAP2 )
		this:explode( this.lights, SHATTER + BITMAP4 )
		this:explode( this.nano2, BITMAPONLY + BITMAP1 )
		this:explode( this.nanobox1, BITMAPONLY + BITMAP2 )
		this:explode( this.pad, BITMAPONLY + BITMAP4 )
		this:explode( this.radar, BITMAPONLY + BITMAP5 )
		this:explode( this.beam2, BITMAPONLY + BITMAP1 )
		return 1
	elseif severity <= 50 then
		this:explode( this.base, BITMAPONLY + BITMAP1 )
		this:explode( this.building1, BITMAPONLY + BITMAP2 )
		this:explode( this.building2, BITMAPONLY + BITMAP3 )
		this:explode( this.lights, SHATTER + BITMAP4 )
		this:explode( this.nano1, BITMAPONLY + BITMAP5 )
		this:explode( this.nano2, BITMAPONLY + BITMAP1 )
		this:explode( this.nanobox1, BITMAPONLY + BITMAP2 )
		this:explode( this.nanobox2, BITMAPONLY + BITMAP3 )
		this:explode( this.pad, BITMAPONLY + BITMAP4 )
		this:explode( this.radar, BITMAPONLY + BITMAP5 )
		this:explode( this.beam1, BITMAPONLY + BITMAP1 )
		this:explode( this.beam2, FALL + FIRE + SMOKE + EXPLODE_ON_HIT + BITMAP2 )
		return 2
	elseif severity <= 99 then
		this:explode( this.base, BITMAPONLY + BITMAP1 )
		this:explode( this.building1, BITMAPONLY + BITMAP2 )
		this:explode( this.building2, BITMAPONLY + BITMAP3 )
		this:explode( this.lights, SHATTER + BITMAP5 )
		this:explode( this.nano1, SHATTER + EXPLODE_ON_HIT + BITMAP5 )
		this:explode( this.nano2, BITMAPONLY + BITMAP5 )
		this:explode( this.nanobox1, BITMAPONLY + BITMAP2 )
		this:explode( this.nanobox2, BITMAPONLY + BITMAP3 )
		this:explode( this.pad, BITMAPONLY + BITMAP4 )
		this:explode( this.radar, FALL + FIRE + SMOKE + EXPLODE_ON_HIT + BITMAP5 )
		this:explode( this.beam1, FALL + FIRE + SMOKE + EXPLODE_ON_HIT + BITMAP5 )
		this:explode( this.beam2, FALL + FIRE + SMOKE + EXPLODE_ON_HIT + BITMAP5 )
		return 3
	end
	this:explode( base, BITMAPONLY + BITMAP1 )
	this:explode( building1, BITMAPONLY + BITMAP2 )
	this:explode( building2, BITMAPONLY + BITMAP3 )
	this:explode( lights, SHATTER + BITMAP5 )
	this:explode( nano1, SHATTER + EXPLODE_ON_HIT + BITMAP5 )
	this:explode( nano2, BITMAPONLY + BITMAP5 )
	this:explode( nanobox1, BITMAPONLY + BITMAP2 )
	this:explode( nanobox2, BITMAPONLY + BITMAP3 )
	this:explode( pad, BITMAPONLY + BITMAP4 )
	this:explode( radar, FALL + FIRE + SMOKE + EXPLODE_ON_HIT + BITMAP5 )
	this:explode( beam1, FALL + FIRE + SMOKE + EXPLODE_ON_HIT + BITMAP5 )
	this:explode( beam2, FALL + FIRE + SMOKE + EXPLODE_ON_HIT + BITMAP5 )
	return 3
end
