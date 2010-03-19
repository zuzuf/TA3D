-- CORREAP

createUnitScript("correap")

__this:piece( "base", "body", "turret", "sleeve1", "sleeve2", "barrel1", "barrel2", "firepoint1", "firepoint2", "smoke1", "smoke2",
			"tracks1", "tracks2", "tracks3", "tracks4", "wheels1", "wheels2", "wheels3", "wheels4", "wheels5", "wheels6", "wheels7" )
			
__this.moving = false
__this.once = 0
__this.animCount= 0
__this.gun1 = 0

local ANIM_SPEED = 0.05
local RESTORE_DELAY = 3.0

local TURRET_TURN_SPEED = 90
local GUN_TURN_SPEED = 50

local WHEEL_TURN_SPEED1 = 960
local WHEEL_TURN_SPEED1_ACCELERATION = 30
local WHEEL_TURN_SPEED1_DECELERATION = 60

local WHEEL_TURN_SPEED2 = 480
local WHEEL_TURN_SPEED2_ACCELERATION = 15
local WHEEL_TURN_SPEED2_DECELERATION = 30

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
	this:turn( this.sleeve1, x_axis, 0, GUN_TURN_SPEED )
	this:turn( this.sleeve2, x_axis, 0, GUN_TURN_SPEED )
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
	
	this:spin( this.wheels2, x_axis, WHEEL_TURN_SPEED1, WHEEL_TURN_SPEED1_ACCELERATION )
	this:spin( this.wheels4, x_axis, WHEEL_TURN_SPEED1, WHEEL_TURN_SPEED1_ACCELERATION )
	this:spin( this.wheels6, x_axis, WHEEL_TURN_SPEED1, WHEEL_TURN_SPEED1_ACCELERATION )

	this:spin( this.wheels1, x_axis, WHEEL_TURN_SPEED2, WHEEL_TURN_SPEED2_ACCELERATION )
	this:spin( this.wheels3, x_axis, WHEEL_TURN_SPEED2, WHEEL_TURN_SPEED2_ACCELERATION )
	this:spin( this.wheels5, x_axis, WHEEL_TURN_SPEED2, WHEEL_TURN_SPEED2_ACCELERATION )
	this:spin( this.wheels7, x_axis, WHEEL_TURN_SPEED2, WHEEL_TURN_SPEED2_ACCELERATION )
end

__this.StopMoving = function(this)
	this.moving = false
	
	-- I don't like insta braking. It's not perfect but works for most cases.
	-- Probably looks goofy when the unit is turtling around, i.e. does not become faster as time increases..
	this.once = this.animCount * ANIM_SPEED / 1000
	if once > 3 then
        once = 3
    end

	this:stop_spin( this.wheels2, x_axis, WHEEL_TURN_SPEED1_DECELERATION )
	this:stop_spin( this.wheels4, x_axis, WHEEL_TURN_SPEED1_DECELERATION )
	this:stop_spin( this.wheels6, x_axis, WHEEL_TURN_SPEED1_DECELERATION )

	this:stop_spin( this.wheels1, x_axis, WHEEL_TURN_SPEED2_DECELERATION )
	this:stop_spin( this.wheels3, x_axis, WHEEL_TURN_SPEED2_DECELERATION )
	this:stop_spin( this.wheels5, x_axis, WHEEL_TURN_SPEED2_DECELERATION )
	this:stop_spin( this.wheels7, x_axis, WHEEL_TURN_SPEED2_DECELERATION )
end

-- Weapons
__this.AimFromPrimary = function(this, piecenum)
	return this.turret
end

__this.QueryPrimary = function(this, piecenum)
	if this.gun1 then
		return this.firepoint1
	else 
		return this.firepoint2
	end
end

__this.AimPrimary = function(this, heading, pitch)
    this:set_script_value("AimPrimary", false)
    heading = heading * TA2DEG
    pitch = pitch * TA2DEG

	this:turn( this.turret, y_axis, heading, TURRET_TURN_SPEED )
	this:turn( this.sleeve1, x_axis, -pitch, GUN_TURN_SPEED )
	this:turn( this.sleeve2, x_axis, -pitch, GUN_TURN_SPEED )

    if this.aiming then
        return
    end
    this.aiming = true

	while this:is_turning( this.turret, y_axis ) or this:is_turning( this.sleeve1, x_axis ) or this:is_turning( this.sleeve2, x_axis ) do
        yield()
    end
    this.aiming = false

    this:start_script( this.RestoreAfterDelay, this, RESTORE_DELAY )
    this:set_script_value("AimPrimary", true)
end

__this.FirePrimary = function(this)
	if this.gun1 then 
		this.gun1 = not this.gun1

		this:emit_sfx( SFXTYPE_WHITESMOKE, this.firepoint1 )
		this:emit_sfx( SFXTYPE_WHITESMOKE, this.smoke1 )
		
		this:move( this.barrel1, z_axis, -2.5, 50 )
		this:sleep( 0.15 )
		this:move( this.barrel1, z_axis, 0, 5 )
	else 
		this.gun1 = not this.gun1

		this:emit_sfx( SFXTYPE_WHITESMOKE, this.firepoint2 )
		this:emit_sfx( SFXTYPE_WHITESMOKE, this.smoke2 )

		this:move( this.barrel2, z_axis, -2.5, 50 )
		this:sleep( 0.15 )
		this:move( this.barrel2, z_axis, 0, 5 )
	end
end

__this.Killed = function(this, severity)
	if severity >= 0 and severity < 25 then
		this:explode( this.barrel1, BITMAPONLY + BITMAP )
		this:explode( this.barrel2, BITMAPONLY + BITMAP )
		this:explode( this.sleeve1, BITMAPONLY + BITMAP )
		this:explode( this.sleeve2, BITMAPONLY + BITMAP )
		this:explode( this.body, BITMAPONLY + BITMAP )
		this:explode( this.turret, BITMAPONLY + BITMAP )
        return 1
	elseif severity >= 25 and severity < 50 then
		this:explode( this.barrel1, FALL + BITMAP )
		this:explode( this.barrel2, FALL + BITMAP )
		this:explode( this.sleeve1, SHATTER + BITMAP )
		this:explode( this.sleeve2, FALL + BITMAP )
		this:explode( this.body, BITMAPONLY + BITMAP )
		this:explode( this.turret, SHATTER + BITMAP )
        return 2
	elseif severity >= 50 and severity < 100 then
		this:explode( this.barrel1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.barrel2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.sleeve1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.sleeve2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.body, BITMAPONLY + BITMAP )
		this:explode( this.turret, SHATTER + BITMAP )
        return 3
	-- D-Gunned/Self-D
	elseif severity >= 100 then
		this:explode( this.barrel1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.barrel2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.sleeve1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.sleeve2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.body, SHATTER + BITMAP )
		this:explode( this.turret, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		return 3
	end
end

__this.Create = function(this)
	this.moving = false
	this.gun1 = false
    this.restoreCount = 0
    this.aiming = false
	
	this:hide( this.tracks1 )
	this:hide( this.tracks2 )
	this:hide( this.tracks3 )
	this:turn_piece_now( this.smoke1, y_axis, -90 )
	this:turn_piece_now( this.smoke2, y_axis, 90 )

	while this:get(BUILD_PERCENT_LEFT) > 0 do
		this:sleep( 0.25 )
	end
	
	this:start_script( this.AnimationControl, this )
    this:start_script( this.SmokeUnit, this )
end
