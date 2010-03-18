-- Core Vehicle Plant Script

createUnitScript("corvp")

__this:piece( "base", "body", "pad", "strobe", "top", "top1", "top2", "top3", "mid", "mid1", "mid2", "mid3",
       "door1", "door2", "bay1a", "bay1b", "bay2a", "bay2b",
       "turret1", "turret2", "turret3", "turret4", "nano1", "nano2", "nano3", "nano4",
       "bottom", "bottom1", "bottom2", "bottom3", "fan1", "fan2", "fan3",
       "beam1", "beam2", "beam3", "beam4" )
       
__this.spray = 0

local SIG_BUILD = 1

local TURNSPEED1 = 180
local TURNSPEED2 = 240
local TURNSPEED3 = 360

local FANSPEED1 = 180
local FANSPEED2 = 1200
local FANRATE   = 45

local MOVESPEED1 = 10
local MOVESPEED2 = 15
local MOVESPEED3 = 20


local BAY_Z = 3.8885
local TURRET_X = 1.2

__this.SMOKEPIECE1 = __this.base

#include "exptype.lh"
#include "StateChg.lh"
#include "smokeunit.lh"
#include "yard.lh"

__this.Open = function(this)
	this:turn( this.top, y_axis, -90, TURNSPEED2 )
	this:wait_for_turn( this.top, y_axis )
	this:turn( this.mid, y_axis, 90, TURNSPEED2 )
	this:wait_for_turn( this.mid, y_axis )
	this:turn( this.bottom, y_axis, 180.01, TURNSPEED3 )
	this:wait_for_turn( this.bottom, y_axis )

	this:move( this.top1, z_axis, 6.5, MOVESPEED2 )
	this:move( this.top3, z_axis, -6.5, MOVESPEED2 )
	this:move( this.mid1, z_axis, -6.5, MOVESPEED2 )
	this:move( this.mid3, z_axis, 6.5, MOVESPEED2 )
	this:move( this.bottom1, x_axis, -6.5, MOVESPEED2 )
	this:move( this.bottom3, x_axis, 6.5, MOVESPEED2 )
	this:move( this.top2, x_axis, 15,4345, MOVESPEED3 )
	this:sleep( 0.25 )
	this:move( this.mid2, x_axis, -15,4345, MOVESPEED3 )
	this:sleep( 0.25 )
	this:move( this.bottom2, z_axis, 15,4345, MOVESPEED3 )

	this:wait_for_move( this.top1, z_axis )
	this:wait_for_move( this.top2, x_axis )
	this:wait_for_move( this.top3, z_axis )
	this:wait_for_move( this.mid1, z_axis )
	this:wait_for_move( this.mid2, x_axis )
	this:wait_for_move( this.mid3, z_axis )
	this:wait_for_move( this.bottom1, x_axis )
	this:wait_for_move( this.bottom2, z_axis )
	this:wait_for_move( this.bottom3, x_axis )
	
	this:turn( this.bay1a, y_axis, -90, TURNSPEED1 )
	this:turn( this.bay2a, y_axis, 90, TURNSPEED1 )
	this:turn( this.door1, y_axis, -179.01, TURNSPEED3 )
	this:turn( this.door2, y_axis, 179.01, TURNSPEED3 )
	this:wait_for_turn( this.bay1a, y_axis )
	this:wait_for_turn( this.bay2a, y_axis )
	this:wait_for_turn( this.door1, y_axis )
	this:wait_for_turn( this.door2, y_axis )
	
	this:move( this.bay1a, z_axis, BAY_Z, MOVESPEED1 )
	this:move( this.bay2a, z_axis, -BAY_Z, MOVESPEED1 )
	this:move( this.turret1, z_axis, 3, MOVESPEED1 )
	this:move( this.turret4, z_axis, -3, MOVESPEED1 )
	this:sleep( 0.1 )

	this:turn( this.bay1b, y_axis, -135, TURNSPEED2 )
	this:turn( this.bay2b, y_axis, 135, TURNSPEED2 )

	this:wait_for_move( this.turret1, z_axis )
	this:wait_for_move( this.turret4, z_axis )
	
	this:move( this.turret2, x_axis, TURRET_X, MOVESPEED1 )
	this:move( this.turret3, x_axis, TURRET_X, MOVESPEED1 )
	this:turn( this.nano2, y_axis, 90, TURNSPEED1 )
	this:turn( this.nano3, y_axis, -90, TURNSPEED1 )
	this:turn( this.nano2, z_axis, -10, TURNSPEED1 )
	this:turn( this.nano3, z_axis, -10, TURNSPEED1 )
	this:turn( this.nano1, x_axis, 15, TURNSPEED1 )
	this:turn( this.nano4, x_axis, -15, TURNSPEED1 )
	this:wait_for_turn( this.nano2, y_axis )
	this:wait_for_turn( this.nano3, y_axis )
	this:wait_for_turn( this.nano1, x_axis )
	this:wait_for_turn( this.nano4, x_axis )
end

__this.Close = function(this)
	this:move( this.turret2, x_axis, 0, MOVESPEED2 )
	this:move( this.turret3, x_axis, 0, MOVESPEED2 )
	this:turn( this.nano2, y_axis, 0, TURNSPEED1 )
	this:turn( this.nano3, y_axis, 0, TURNSPEED1 )
	this:turn( this.nano2, z_axis, 0, TURNSPEED1 )
	this:turn( this.nano3, z_axis, 0, TURNSPEED1 )
	this:turn( this.nano1, x_axis, 0, TURNSPEED1 )
	this:turn( this.nano4, x_axis, 0, TURNSPEED1 )
	this:wait_for_turn( this.nano2, y_axis )
	this:wait_for_turn( this.nano3, y_axis )
	this:wait_for_turn( this.nano1, x_axis )
	this:wait_for_turn( this.nano4, x_axis )
		
	this:turn( this.bay1b, y_axis, 0, TURNSPEED1 )
	this:turn( this.bay2b, y_axis, 0, TURNSPEED1 )
	this:wait_for_turn( this.bay1b, y_axis )
	this:wait_for_turn( this.bay2b, y_axis )
	this:sleep( 0.1 )

	this:move( this.bay1a, z_axis, 0, MOVESPEED1 )
	this:move( this.bay2a, z_axis, 0, MOVESPEED1 )
	this:move( this.turret1, z_axis, 0, MOVESPEED1 )
	this:move( this.turret4, z_axis, 0, MOVESPEED1 )

	this:wait_for_move( this.turret1, z_axis )
	this:wait_for_move( this.turret4, z_axis )


	this:turn( this.bay1a, y_axis, 0, TURNSPEED1 )
	this:turn( this.bay2a, y_axis, 0, TURNSPEED1 )
	this:turn( this.door1, y_axis, 0, TURNSPEED3 )
	this:turn( this.door2, y_axis, 0, TURNSPEED3 )
	this:wait_for_turn( this.bay1a, y_axis )
	this:wait_for_turn( this.bay2a, y_axis )
	this:wait_for_turn( this.door1, y_axis )
	this:wait_for_turn( this.door2, y_axis )

	this:move( this.top1, z_axis,    0, MOVESPEED2 )
	this:move( this.top3, z_axis,    0, MOVESPEED2 )
	this:move( this.mid1, z_axis,    0, MOVESPEED2 )
	this:move( this.mid3, z_axis,    0, MOVESPEED2 )
	this:move( this.bottom1, x_axis, 0, MOVESPEED2 )
	this:move( this.bottom3, x_axis, 0, MOVESPEED2 )

	this:move( this.bottom2, z_axis, 0, MOVESPEED3 )
	this:sleep( 0.25 )
	this:move( this.mid2, x_axis, 0, MOVESPEED3 )
	this:sleep( 0.25 )
	this:move( this.top2, x_axis, 0, MOVESPEED3 )

	this:wait_for_move( this.top1, z_axis )
	this:wait_for_move( this.top2, z_axis )
	this:wait_for_move( this.top3, z_axis )
	this:wait_for_move( this.mid1, z_axis )
	this:wait_for_move( this.mid2, z_axis )
	this:wait_for_move( this.mid3, z_axis )
	this:wait_for_move( this.bottom1, x_axis )
	this:wait_for_move( this.bottom2, z_axis )
	this:wait_for_move( this.bottom3, x_axis )

	this:turn( this.bottom, y_axis, 0, TURNSPEED3 )
	this:wait_for_turn( this.bottom, y_axis )
	this:turn( this.mid, y_axis, 0, TURNSPEED2 )
	this:wait_for_turn( this.mid, y_axis )
	this:turn( this.top, y_axis, 0, TURNSPEED2 )
	this:wait_for_turn( this.top, y_axis )
end

__this.Go = function (this)
    this:Open()
    this:OpenYard()
    this:set( INBUILDSTANCE, true )
end

__this.Stop = function(this)
    this:set( INBUILDSTANCE, false )
    this:CloseYard()
    this:Close()
end

__this.ACTIVATECMD = __this.Go
__this.DEACTIVATECMD = __this.Stop

__this.Activate = function(this)
    this:start_script( this.RequestState, this, ACTIVE )
end

__this.Deactivate = function(this)
    this:start_script( this.RequestState, this, INACTIVE )
end

__this.Create = function(this)
	this.spray = 0
	this:spin( this.fan1, x_axis, FANSPEED1, FANRATE )
	this:spin( this.fan2, z_axis, FANSPEED1, FANRATE )
	this:spin( this.fan3, x_axis, FANSPEED1, FANRATE )
    this:InitState()
    this:start_script( this.SmokeUnit, this )
end

__this.QueryNanoPiece = function(this)
    this.spray = (this.spray + 1) % 4
	if this.spray == 1 then
		return this.beam1
	elseif this.spray == 2 then
		return this.beam2
	elseif this.spray == 3 then
		return this.beam3
	elseif this.spray == 0 then
		return this.beam4
	end

    return 0
end

__this.StartBuilding = function(this, heading, pitch)
	this:signal( SIG_BUILD )
	this:set_signal_mask( SIG_BUILD )

    heading = heading * TA2DEG
    pitch = pitch * TA2DEG
    
	this:spin( this.fan1, x_axis, FANSPEED2, FANRATE )
	this:spin( this.fan2, z_axis, FANSPEED2, FANRATE )
	this:spin( this.fan3, x_axis, FANSPEED2, FANRATE )
	
	this:turn( this.nano2, z_axis, -pitch, TURNSPEED1 )
	this:turn( this.nano3, z_axis, -pitch, TURNSPEED1 )
	this:turn( this.nano1, x_axis, -pitch, TURNSPEED1 )
	this:turn( this.nano4, x_axis, -pitch, TURNSPEED1 )

	this:move_piece_now( this.pad, z_axis, 0 )
	this:move( this.pad, z_axis, 5, 0.25 )

	while true do
		local offset1 = math.random(-1500, 1500) * 0.01
		local offset2 = math.random(-100, 100) * 0.01

		this:turn( this.nano2, y_axis, heading + offset1 + 90, TURNSPEED1 )
		this:turn( this.nano3, y_axis, heading + offset1 - 90, TURNSPEED1 )
		this:turn( this.nano1, y_axis, heading + offset1     , TURNSPEED1 )
		this:turn( this.nano4, y_axis, heading - offset1     , TURNSPEED1 )
		if math.random(0,1) == 0 then
			this:move( this.turret2, x_axis, TURRET_X + offset2, MOVESPEED1 )
			this:move( this.turret3, x_axis, TURRET_X - offset2, MOVESPEED1 )
		end

		this:sleep( math.random(150,500) * 0.001 )
    end
end

__this.StopBuilding = function(this)
	this:signal( SIG_BUILD )
	this:set_signal_mask( SIG_BUILD )
	
	this:move_piece_now( this.pad, z_axis, 0 )
	this:stop_spin( this.fan1, x_axis, FANRATE )
	this:stop_spin( this.fan2, z_axis, FANRATE )
	this:stop_spin( this.fan3, x_axis, FANRATE )
	this:sleep( 0.5 )
	this:spin( this.fan1, x_axis, FANSPEED1, -FANRATE )
	this:spin( this.fan2, z_axis, FANSPEED1, -FANRATE )
	this:spin( this.fan3, x_axis, FANSPEED1, -FANRATE )
end

__this.QueryBuildInfo = function(this)
	return this.pad
end

__this.Killed = function(this, severity)
	if severity >= 0 and severity < 25 then
		this:explode( this.body, BITMAPONLY + BITMAP )
		this:explode( this.top1, BITMAPONLY + BITMAP )
		this:explode( this.top2, BITMAPONLY + BITMAP )
		this:explode( this.top3, BITMAPONLY + BITMAP )
		this:explode( this.mid1, BITMAPONLY + BITMAP )
		this:explode( this.mid2, BITMAPONLY + BITMAP )
		this:explode( this.mid3, BITMAPONLY + BITMAP )
		this:explode( this.bottom1, BITMAPONLY + BITMAP )
		this:explode( this.bottom2, BITMAPONLY + BITMAP )
		this:explode( this.bottom3, BITMAPONLY + BITMAP )
        return 1
	elseif severity >= 25 and severity < 50 then
		this:explode( this.body, BITMAPONLY + BITMAP )
		this:explode( this.top1, BITMAPONLY + BITMAP )
		this:explode( this.top2, BITMAPONLY + BITMAP )
		this:explode( this.top3, BITMAPONLY + BITMAP )
		this:explode( this.mid1, BITMAPONLY + BITMAP )
		this:explode( this.mid2, BITMAPONLY + BITMAP )
		this:explode( this.mid3, BITMAPONLY + BITMAP )
		this:explode( this.bottom1, BITMAPONLY + BITMAP )
		this:explode( this.bottom2, BITMAPONLY + BITMAP )
		this:explode( this.bottom3, BITMAPONLY + BITMAP )
		this:explode( this.bay1a, FALL + BITMAP )
		this:explode( this.bay2a, FALL + BITMAP )
		this:explode( this.bay1b, FALL + BITMAP )
		this:explode( this.bay2b, FALL + BITMAP )
        return 2
	elseif severity >= 50 and severity < 100 then
		this:explode( this.body, BITMAPONLY + BITMAP )
		this:explode( this.top1, BITMAPONLY + BITMAP )
		this:explode( this.top2, BITMAPONLY + BITMAP )
		this:explode( this.top3, BITMAPONLY + BITMAP )
		this:explode( this.mid1, BITMAPONLY + BITMAP )
		this:explode( this.mid2, BITMAPONLY + BITMAP )
		this:explode( this.mid3, BITMAPONLY + BITMAP )
		this:explode( this.bottom1, BITMAPONLY + BITMAP )
		this:explode( this.bottom2, BITMAPONLY + BITMAP )
		this:explode( this.bottom3, BITMAPONLY + BITMAP )
		this:explode( this.bay1a, FALL + SMOKE + FIRE + EXPLODE_ON_HIT )
		this:explode( this.bay2a, FALL + SMOKE + FIRE + EXPLODE_ON_HIT )
		this:explode( this.bay1b, FALL + SMOKE + FIRE + EXPLODE_ON_HIT )
		this:explode( this.bay2b, FALL + SMOKE + FIRE + EXPLODE_ON_HIT )
		this:explode( this.door2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT )
		this:explode( this.door1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT )
		return 3
	-- D-Gunned/Self-D
	elseif severity >= 100 then
		this:explode( this.body, SHATTER + BITMAP )
		this:explode( this.top1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT )
		this:explode( this.top2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT )
		this:explode( this.top3, FALL + SMOKE + FIRE + EXPLODE_ON_HIT )
		this:explode( this.mid1, SHATTER + BITMAP )
		this:explode( this.mid2, SHATTER + BITMAP )
		this:explode( this.mid3, SHATTER + BITMAP )
		this:explode( this.bottom1, BITMAPONLY + BITMAP )
		this:explode( this.bottom2, BITMAPONLY + BITMAP )
		this:explode( this.bottom3, BITMAPONLY + BITMAP )
		this:explode( this.bay1a, FALL + SMOKE + FIRE + EXPLODE_ON_HIT )
		this:explode( this.bay2a, FALL + SMOKE + FIRE + EXPLODE_ON_HIT )
		this:explode( this.bay1b, FALL + SMOKE + FIRE + EXPLODE_ON_HIT )
		this:explode( this.bay2b, FALL + SMOKE + FIRE + EXPLODE_ON_HIT )
		this:explode( this.door2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT )
		this:explode( this.door1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT )
        return 3
    end
end
