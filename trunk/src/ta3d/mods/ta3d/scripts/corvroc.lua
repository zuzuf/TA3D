-- CORVROC

createUnitScript("corvroc")

__this:piece( "base", "missle", "gantry", "wheels1", "wheels2", "wheels3", "wheels4", "wheels5", "wheels6", "tracks1", "tracks2", "tracks3", "tracks4" )

__this.SMOKEPIECE1 = __this.base

local ANIM_SPEED = 0.05

local WHEEL_TURN_SPEED1 = 480
local WHEEL_TURN_SPEED1_ACCELERATION = 75
local WHEEL_TURN_SPEED1_DECELERATION = 200

local WHEEL_TURN_SPEED2 = 240
local WHEEL_TURN_SPEED2_ACCELERATION = 45
local WHEEL_TURN_SPEED2_DECELERATION = 120

#include "smokeunit.lh"
#include "hitweap.lh"
#include "StateChg.lh"

__this.AnimationControl = function(this)
    local current_tracks = 0
    
    while true do
        if this.moving or this.once > 0 then
            if current_tracks == 0 then
                this:show( this.tracks1 )
                this:hide( this.tracks4 )
                current_tracks = 1
            elseif current_tracks == 1 then
                this:show( this.tracks2 )
                this:hide( this.tracks1 )
                current_tracks = 2
            elseif current_tracks == 2 then
                this:show( this.tracks3 )
                this:hide( this.tracks2 )
                current_tracks = 3
            elseif current_tracks == 3 then
                this:show( this.tracks4 )
                this:hide( this.tracks3 )
                current_tracks = 0
                if this.once > 0 then
                    this.once = this.once - 1
                end
            end
            this.animCount = this.animCount + 1
        end
        this:sleep( ANIM_SPEED )
    end
end

__this.StartMoving = function(this)
    this.moving = true
    this.animCount = 0
    
    this:spin( this.wheels1, x_axis, WHEEL_TURN_SPEED2, WHEEL_TURN_SPEED2_ACCELERATION )
    this:spin( this.wheels6, x_axis, WHEEL_TURN_SPEED2, WHEEL_TURN_SPEED2_ACCELERATION )

    this:spin( this.wheels2, x_axis, WHEEL_TURN_SPEED1, WHEEL_TURN_SPEED1_ACCELERATION )
    this:spin( this.wheels3, x_axis, WHEEL_TURN_SPEED1, WHEEL_TURN_SPEED1_ACCELERATION )
    this:spin( this.wheels4, x_axis, WHEEL_TURN_SPEED1, WHEEL_TURN_SPEED1_ACCELERATION )
    this:spin( this.wheels5, x_axis, WHEEL_TURN_SPEED1, WHEEL_TURN_SPEED1_ACCELERATION )
end

__this.StopMoving = function(this)
    this.moving = false
    
    -- I don't like insta braking. It's not perfect but works for most cases.
    -- Probably looks goofy when the unit is turtling around, i.e. does not become faster as time increases..
    this.once = this.animCount * ANIM_SPEED / 1000
    if this.once > 3 then
        this.once = 3
    end

    this:stop_spin( this.wheels1, x_axis, WHEEL_TURN_SPEED2_DECELERATION )
    this:stop_spin( this.wheels6, x_axis, WHEEL_TURN_SPEED2_DECELERATION )

    this:stop_spin( this.wheels2, x_axis, WHEEL_TURN_SPEED1_DECELERATION )
    this:stop_spin( this.wheels3, x_axis, WHEEL_TURN_SPEED1_DECELERATION )
    this:stop_spin( this.wheels4, x_axis, WHEEL_TURN_SPEED1_DECELERATION )
    this:stop_spin( this.wheels5, x_axis, WHEEL_TURN_SPEED1_DECELERATION )
end

__this.activatescr = function(this)
	this:turn_piece_now( this.gantry, x_axis, 0.0 )
	this:turn( this.gantry, x_axis, -90.0, 45.395604 )
    this:wait_for_turn( this.gantry, x_axis )
end

__this.deactivatescr = function(this)
	this:turn_piece_now( this.gantry, x_axis, -90.0 )
	this:turn( this.gantry, x_axis, 0.0, 45.098901 )
	this:wait_for_turn( this.gantry, x_axis )
end

__this.Go = function(this)
    this.gun_1 = false
	this:show( this.missle )
	this:activatescr()
	this.gun_1 = true
end

__this.Stop = function(this)
	this.gun_1 = false
	this:deactivatescr()
end

__this.ACTIVATECMD     = __this.Go
__this.DEACTIVATECMD   = __this.Stop

__this.Create = function(this)
    this.moving = false
    this.once = 0
    this.animCount = 0
    this.aiming = false
    this.gun_1 = false
    this.restore_delay = 3.0
    this.restoreCount = 0

	this:dont_shade( this.missle )
	this:dont_cache( this.missle )
	this:InitState()
    while this:get(BUILD_PERCENT_LEFT) > 0 do
        this:sleep( 0.25 )
    end
    this:start_script( this.SmokeUnit, this )
    this:start_script( this.AnimationControl, this )
end

__this.RestoreAfterDelay = function(this)
    this.restoreCount = this.restoreCount + 1
    local ncall = this.restoreCount
    this:sleep( this.restore_delay )
    if ncall ~= this.restoreCount then
        return
    end
	this:start_script( this.RequestState, this, INACTIVE )
end

__this.AimPrimary = function(this, heading, pitch)
    this:set_script_value("AimPrimary", false)
    if this.aiming then
        return
    end
    this.aiming = true
	this:start_script( this.RequestState, this, ACTIVE )

	while not this.gun_1 do
		this:sleep( 0.25 )
	end

    this:set_script_value("AimPrimary", true)
    this.aiming = false

	this:start_script( this.RestoreAfterDelay, this )
end

__this.FirePrimary = function(this)
	this:hide( this.missle )
	this.gun_1 = false
	this:start_script( this.RequestState, this, INACTIVE )
end

__this.QueryPrimary = function(this)
    return this.missle
end

__this.SweetSpot = function(this)
	return this.base
end

__this.Killed = function(this, severity)
	if severity <= 25 then
		this:explode( this.base, BITMAPONLY + BITMAP3 )
		this:explode( this.gantry, BITMAPONLY + BITMAP4 )
		this:explode( this.missle, BITMAPONLY + BITMAP5 )
		return 1
	elseif severity <= 50 then
		this:explode( this.base, BITMAPONLY + BITMAP3 )
		this:explode( this.gantry, BITMAPONLY + BITMAP4 )
		this:explode( this.missle, SHATTER + BITMAP5 )
		return 2
	elseif severity <= 99 then
		this:explode( this.base, BITMAPONLY + BITMAP3 )
		this:explode( this.gantry, BITMAPONLY + BITMAP4 )
		this:explode( this.missle, SHATTER + BITMAP5 )
        return 3
    end
	this:explode( this.base, BITMAPONLY + BITMAP3 )
	this:explode( this.gantry, BITMAPONLY + BITMAP4 )
	this:explode( this.missle, SHATTER + EXPLODE_ON_HIT + BITMAP5 )
    return 3
end
