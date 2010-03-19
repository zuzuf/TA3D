-- Arm Mobile SAM Launcher

createUnitScript("cormist")

__this:piece( "base", "body", "turret", "arms", "firepoint1", "firepoint2", "exhaust1", "exhaust2", "gun",
			"tracks1", "tracks2", "tracks3", "tracks4",
			"wheels1", "wheels2", "wheels3", "wheels4", "wheels5", "wheels6", "wheels7", "wheels8" )

__this.moving = false
__this.once = 0
__this.animCount = 0
__this.curGun = 0
__this.deployed = false
__this.firepoint = __this.firepoint1
__this.aiming = false

-- Signal definitions
local ANIM_SPEED = 0.05
local RESTORE_DELAY = 3.0

local TURRET_TURN_SPEED = 240
local GUN_TURN_SPEED = 60
local ARMS_MOVE_SPEED = 18

local WHEEL_TURN_SPEED1 = 240
local WHEEL_TURN_SPEED1_ACCELERATION = 45
local WHEEL_TURN_SPEED1_DECELERATION = 120

local WHEEL_TURN_SPEED2 = 480
local WHEEL_TURN_SPEED2_ACCELERATION = 75
local WHEEL_TURN_SPEED2_DECELERATION = 200

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
	this:turn( this.turret, y_axis, 0, TURRET_TURN_SPEED/2 )
	this:turn( this.gun, x_axis, 0, GUN_TURN_SPEED/2 )
	this:move( this.arms, y_axis, 0, ARMS_MOVE_SPEED/2 )
	this.deployed = false
end

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
	
	this:spin( this.wheels2, x_axis, WHEEL_TURN_SPEED2, WHEEL_TURN_SPEED2_ACCELERATION )
	this:spin( this.wheels3, x_axis, WHEEL_TURN_SPEED2, WHEEL_TURN_SPEED2_ACCELERATION )

	this:spin( this.wheels1, x_axis, WHEEL_TURN_SPEED1, WHEEL_TURN_SPEED1_ACCELERATION )
	this:spin( this.wheels4, x_axis, WHEEL_TURN_SPEED1, WHEEL_TURN_SPEED1_ACCELERATION )
	this:spin( this.wheels5, x_axis, WHEEL_TURN_SPEED1, WHEEL_TURN_SPEED1_ACCELERATION )
	this:spin( this.wheels6, x_axis, WHEEL_TURN_SPEED1, WHEEL_TURN_SPEED1_ACCELERATION )
	this:spin( this.wheels7, x_axis, WHEEL_TURN_SPEED1, WHEEL_TURN_SPEED1_ACCELERATION )
	this:spin( this.wheels8, x_axis, WHEEL_TURN_SPEED1, WHEEL_TURN_SPEED1_ACCELERATION )
end

__this.StopMoving = function (this)
	this.moving = false
	
	-- I don't like insta braking.It's not perfect but works for most cases.
	-- Probably looks goofy when the unit is turtling around, i.e.does not get faster as time increases..
	this.once = this.animCount * ANIM_SPEED / 1000
	if this.once > 3 then
        this.once = 3
    end

	this:stop_spin( this.wheels2, x_axis, WHEEL_TURN_SPEED2_DECELERATION )
	this:stop_spin( this.wheels3, x_axis, WHEEL_TURN_SPEED2_DECELERATION )

	this:stop_spin( this.wheels1, x_axis, WHEEL_TURN_SPEED1_DECELERATION )
	this:stop_spin( this.wheels4, x_axis, WHEEL_TURN_SPEED1_DECELERATION )
	this:stop_spin( this.wheels5, x_axis, WHEEL_TURN_SPEED1_DECELERATION )
	this:stop_spin( this.wheels6, x_axis, WHEEL_TURN_SPEED1_DECELERATION )
	this:stop_spin( this.wheels7, x_axis, WHEEL_TURN_SPEED1_DECELERATION )
	this:stop_spin( this.wheels8, x_axis, WHEEL_TURN_SPEED1_DECELERATION )
end

-- Weapons
__this.AimFromPrimary = function(this)
	return this.gun
end

__this.QueryPrimary = function(this)
	return this.firepoint
end

__this.AimPrimary = function(this, heading, pitch)
    this:set_script_value("AimPrimary", false)
    
	heading = heading * TA2DEG
	pitch = pitch * TA2DEG

	this:turn( this.turret, y_axis, heading, TURRET_TURN_SPEED )
	this:turn( this.gun, x_axis, -pitch, GUN_TURN_SPEED )
	
	if this.aiming then
        return
    end
    
    this.aiming = true

	if not this.deployed then
		this:move( this.arms, y_axis, 5.5, ARMS_MOVE_SPEED )
		this:wait_for_move( this.arms, y_axis )
		this.deployed = true
	end

    while this:is_turning( this.turret, y_axis ) or this:is_turning( this.gun, y_axis ) do
        yield()
    end

    this.aiming = false
	this:start_script( this.RestoreAfterDelay, this, RESTORE_DELAY )
	
    this:set_script_value("AimPrimary", true)
end

__this.FirePrimary = function(this)
	if not this.curGun then
		this.firepoint = this.firepoint1
	else
		this.firepoint = this.firepoint2
	end
	this.curGun = not this.curGun
end

__this.Killed = function(this, severity)
	if severity >= 0 and severity < 25 then
		this:explode( this.gun, BITMAPONLY + BITMAP )
		this:explode( this.turret, BITMAPONLY + BITMAP )
		this:explode( this.body, BITMAPONLY + BITMAP )
		return 1
	elseif severity >= 25 and severity < 50 then
		this:explode( this.gun, FALL + BITMAP )
		this:explode( this.turret, SHATTER + BITMAP )
		this:explode( this.body, BITMAPONLY + BITMAP )
		return 2
	elseif severity >= 50 and severity < 100 then
		this:explode( this.gun, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.turret, SHATTER + BITMAP )
		this:explode( this.body, BITMAPONLY + BITMAP )
		return 3
	-- D-Gunned/Self-D
	elseif severity >= 100 then
		this:explode( this.gun, SHATTER + BITMAP )
		this:explode( this.turret, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.body, SHATTER + BITMAP )
		return 3
	end
	return 0
end

__this.Create = function(this)
	this.moving = false
	this.aiming = false
    this.restoreCount = 0

	this:hide( this.tracks1 )
	this:hide( this.tracks2 )
	this:hide( this.tracks3 )

	this:turn_piece_now( this.exhaust1, y_axis, 180 )
	this:turn_piece_now( this.exhaust2, y_axis, 180 )
	
	this.curGun = 0
	this.deployed = false
	
	while this:get(BUILD_PERCENT_LEFT) > 0 do
		this:sleep( 0.25 )
	end
	this:start_script( this.AnimationControl, this )
    this:start_script( this.SmokeUnit, this )
end
