-- CORRAID

createUnitScript("corraid")

__this:piece( "base", "body", "turret", "sleeve", "barrel", "firepoint",
			"tracks1", "tracks2", "tracks3", "tracks4",
			"wheels1", "wheels2", "wheels3", "wheels4", "wheels5" )

__this.moving = false
__this.once = false
__this.animCount = 0

-- Signal definitions
local ANIM_SPEED = 0.05
local RESTORE_DELAY = 3.0

local TURRET_TURN_SPEED = 90
local SLEEVE_TURN_SPEED = 45

local WHEEL_TURN_SPEED1 = 240
local WHEEL_TURN_SPEED1_ACCELERATION = 45
local WHEEL_TURN_SPEED1_DECELERATION = 120

local WHEEL_TURN_SPEED2 = 360
local WHEEL_TURN_SPEED2_ACCELERATION = 60
local WHEEL_TURN_SPEED2_DECELERATION = 160

__this.SMOKEPIECE1 = __this.body
__this.SMOKEPIECE2 = __this.turret

#include "exptype.lh"
#include "smokeunit.lh"
#include "hitweap.lh"

__this.RestoreAfterDelay = function(this, delay)
    this.restoreCount = this.restoreCount + 1
    local ncall = this.restoreCount
    this:sleep( delay )
    if ncall ~= this.restoreCount then
        return
    end
	this:turn( this.turret, y_axis, 0, TURRET_TURN_SPEED )
end

__this.AnimationControl = function(this)
    local current_tracks = 0
    
    while true do
        if this.moving or this.once then
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
                this.once = false
            end
            this.animCount = this.animCount + 1
        end
        this:sleep( ANIM_SPEED )
    end
end

__this.StartMoving = function(this)
	this.moving = true
	this.animCount = 0
	
	this:spin( this.wheels1, x_axis, WHEEL_TURN_SPEED1, WHEEL_TURN_SPEED1_ACCELERATION )
	this:spin( this.wheels5, x_axis, WHEEL_TURN_SPEED1, WHEEL_TURN_SPEED1_ACCELERATION )
	this:spin( this.wheels2, x_axis, WHEEL_TURN_SPEED2, WHEEL_TURN_SPEED2_ACCELERATION )
	this:spin( this.wheels3, x_axis, WHEEL_TURN_SPEED2, WHEEL_TURN_SPEED2_ACCELERATION )
	this:spin( this.wheels4, x_axis, WHEEL_TURN_SPEED2, WHEEL_TURN_SPEED2_ACCELERATION )
end

__this.StopMoving = function(this)
	this.moving = false
	
	-- I don't like insta braking. It's not perfect but works for most cases.
	-- Probably looks goofy when the unit is turtling around, i.e. does not get faster as time increases..
	this.once = this.animCount * ANIM_SPEED / 1000
	if once > 3 then
        once = 3
    end

	this:stop_spin( this.wheels1, x_axis, WHEEL_TURN_SPEED1_DECELERATION )
	this:stop_spin( this.wheels5, x_axis, WHEEL_TURN_SPEED1_DECELERATION )
	this:stop_spin( this.wheels2, x_axis, WHEEL_TURN_SPEED2_DECELERATION )
	this:stop_spin( this.wheels3, x_axis, WHEEL_TURN_SPEED2_DECELERATION )
	this:stop_spin( this.wheels4, x_axis, WHEEL_TURN_SPEED2_DECELERATION )
end

-- Weapons
__this.AimFromPrimary = function(this)
	return this.turret
end

__this.QueryPrimary = function(this)
	return this.firepoint
end

__this.AimPrimary = function(this, heading, pitch)
    this:set_script_value("AimPrimary", false)
    
    heading = heading * TA2DEG
    pitch = pitch * TA2DEG

	this:turn( this.turret, y_axis, heading, TURRET_TURN_SPEED )
	this:turn( this.sleeve, x_axis, -pitch, SLEEVE_TURN_SPEED )
    
    if this.aiming then
        return
    end
    this.aiming = true

    while this:is_turning( this.turret, y_axis ) or this:is_turning( this.sleeve, y_axis ) do
        yield()
    end
    this.aiming = false

    this:start_script( this.RestoreAfterDelay, this, RESTORE_DELAY )
    this:set_script_value("AimPrimary", true)
end

__this.FirePrimary = function(this)
	this:move_piece_now( this.barrel, z_axis, -3 )
	this:move( this.barrel, z_axis, 0, 3.75 )
	this:wait_for_move( this.barrel, z_axis ) 
end


__this.Killed = function(this, severity)
	if severity >= 0 and severity < 25 then
		this:explode( this.barrel, BITMAPONLY + BITMAP )
		this:explode( this.sleeve, BITMAPONLY + BITMAP )
		this:explode( this.body, BITMAPONLY + BITMAP )
		this:explode( this.turret, BITMAPONLY + BITMAP )
        return 1
	elseif severity >= 25 and severity < 50 then
		this:explode( this.barrel, FALL + BITMAP )
		this:explode( this.sleeve, FALL + BITMAP )
		this:explode( this.body, BITMAPONLY + BITMAP )
		this:explode( this.turret, SHATTER + BITMAP )
        return 2
	elseif severity >= 50 and severity < 100 then
		this:explode( this.barrel, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.sleeve, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.body, BITMAPONLY + BITMAP )
		this:explode( this.turret, SHATTER + BITMAP )
        return 3
	-- D-Gunned/Self-D
	elseif severity >= 100 then
		this:explode( this.barrel, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.sleeve, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.body, SHATTER + BITMAP )
		this:explode( this.turret, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
        return 3
	end
end

__this.Create = function(this)
	this.moving = false
    this.restoreCount = 0
    this.aiming = false

	this:hide( this.tracks1 )
	this:hide( this.tracks2 )
	this:hide( this.tracks3 )
	this:hide( this.firepoint )
	
	while this:get(BUILD_PERCENT_LEFT) > 0 do
		this:sleep( 0.25 )
	end
	
	this:start_script( this.AnimationControl, this )
    this:start_script( this.SmokeUnit, this )
end
