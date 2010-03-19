-- Core Construction Vehicle

createUnitScript("corcv")

__this:piece( "base", "body", "lift", "turret", "cradle", "rear", "nano", "panel_t", "panel_r", "panel_l", "panel_b", 
			"firepoint", "door1", "door2", "tracks1", "tracks2", "tracks3", "tracks4", "wheels1", "wheels2",
			"wheels3", "wheels4", "wheels5", "wheels6", "wheels7" )

__this.moving = false
__this.once = false
__this.animCount = 0

local ANIM_SPEED = 0.05

local TURN_SPEED1 = 160
local TURN_SPEED2 = 240
local MOVE_SPEED1 = 5
local MOVE_SPEED2 = 10
local SLEEP_TIME1 = 0.1
local SLEEP_TIME2 = 0.5

local WHEEL_TURN_SPEED1 = 480
local WHEEL_TURN_SPEED1_ACCELERATION = 75
local WHEEL_TURN_SPEED1_DECELERATION = 200

local DOOR_ANGLE_Z = 160
local PANEL_ANGLE_MIN = 60
local PANEL_ANGLE_MAX = 90
local TURRET_Y = 2.75
local LIFT_Y = 6.25
local REAR_Z = -1.5
local NANO_Z = 1.75

__this.SMOKEPIECE1 = __this.body
__this.SMOKEPIECE2 = __this.turret

#include "exptype.lh"
#include "StateChg.lh"
#include "smokeunit.lh"
#include "hitweap.lh"

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
	this:spin( this.wheels2, x_axis, WHEEL_TURN_SPEED1, WHEEL_TURN_SPEED1_ACCELERATION )
	this:spin( this.wheels3, x_axis, WHEEL_TURN_SPEED1, WHEEL_TURN_SPEED1_ACCELERATION )
	this:spin( this.wheels4, x_axis, WHEEL_TURN_SPEED1, WHEEL_TURN_SPEED1_ACCELERATION )
	this:spin( this.wheels5, x_axis, WHEEL_TURN_SPEED1, WHEEL_TURN_SPEED1_ACCELERATION )
	this:spin( this.wheels6, x_axis, WHEEL_TURN_SPEED1, WHEEL_TURN_SPEED1_ACCELERATION )
	this:spin( this.wheels7, x_axis, WHEEL_TURN_SPEED1, WHEEL_TURN_SPEED1_ACCELERATION )
end

__this.StopMoving = function(this)
	this.moving = false
	
	-- I don't like insta braking. It's not perfect but works for most cases.
	-- Probably looks goofy when the unit is turtling around, i.e. does not become faster as time increases..
	this.once = this.animCount * ANIM_SPEED / 1000

	this:stop_spin( this.wheels1, x_axis, WHEEL_TURN_SPEED1_DECELERATION )
	this:stop_spin( this.wheels2, x_axis, WHEEL_TURN_SPEED1_DECELERATION )
	this:stop_spin( this.wheels3, x_axis, WHEEL_TURN_SPEED1_DECELERATION )
	this:stop_spin( this.wheels4, x_axis, WHEEL_TURN_SPEED1_DECELERATION )
	this:stop_spin( this.wheels5, x_axis, WHEEL_TURN_SPEED1_DECELERATION )
	this:stop_spin( this.wheels6, x_axis, WHEEL_TURN_SPEED1_DECELERATION )
	this:stop_spin( this.wheels7, x_axis, WHEEL_TURN_SPEED1_DECELERATION )
end

__this.Go = function(this)

	this:turn( this.door1, z_axis, -DOOR_ANGLE_Z, TURN_SPEED2 )
	this:turn( this.door2, z_axis, DOOR_ANGLE_Z, TURN_SPEED2 )
	this:sleep( SLEEP_TIME2 )
	
	this:move( this.lift, y_axis, LIFT_Y, MOVE_SPEED2 )
	this:move( this.turret, y_axis, TURRET_Y, MOVE_SPEED2 )
	this:wait_for_move( this.lift, y_axis )
	
	this:move( this.nano, z_axis, NANO_Z, MOVE_SPEED2 )
	this:move( this.rear, z_axis, REAR_Z, MOVE_SPEED2 )
	this:turn( this.panel_t, x_axis, -PANEL_ANGLE_MIN, TURN_SPEED2 )
	this:turn( this.panel_b, x_axis, PANEL_ANGLE_MIN, TURN_SPEED2 )
	this:turn( this.panel_l, y_axis, PANEL_ANGLE_MIN, TURN_SPEED2 )
	this:turn( this.panel_r, y_axis, -PANEL_ANGLE_MIN, TURN_SPEED2 )
	this:turn( this.turret, y_axis, this.buildheading, TURN_SPEED2 )
	this:turn( this.cradle, x_axis, -this.buildpitch, TURN_SPEED2 )
	this:wait_for_turn( this.turret, y_axis )
	this:wait_for_turn( this.cradle, x_axis )

	this:set( INBUILDSTANCE, true )
	
	this:start_script( this.buildMonitor, this )
end

__this.buildMonitor = function(this)
	if this.build_monitor then
        return
    end
	this.build_monitor = true
	while this:get(INBUILDSTANCE) == 1 do
		if math.random(1,100) > 66 then
			local rand_turret_y = math.random(0,2)
			local rand_panel_angle = math.random(0, 1)
			if rand_panel_angle == 1 then
                rand_panel_angle = PANEL_ANGLE_MAX
			else
                rand_panel_angle = PANEL_ANGLE_MIN
            end
			
			this:move( this.turret, y_axis, (TURRET_Y - rand_turret_y), MOVE_SPEED2 )

			this:turn( this.panel_t, x_axis, -rand_panel_angle, TURN_SPEED1 )
			this:turn( this.panel_b, x_axis, rand_panel_angle, TURN_SPEED1 )
			this:turn( this.panel_l, y_axis, rand_panel_angle, TURN_SPEED1 )
			this:turn( this.panel_r, y_axis, -rand_panel_angle, TURN_SPEED1 )
		end
		this:sleep( 0.25 )
	end
    this.build_monitor = false
end

__this.Stop = function(this)
	this:set( INBUILDSTANCE, false )

	this:move( this.nano, z_axis, 0, MOVE_SPEED1 )
	this:move( this.rear, z_axis, 0, MOVE_SPEED1 )
	this:turn( this.panel_t, x_axis, 0, TURN_SPEED1 )
	this:turn( this.panel_b, x_axis, 0, TURN_SPEED1 )
	this:turn( this.panel_l, y_axis, 0, TURN_SPEED1 )
	this:turn( this.panel_r, y_axis, 0, TURN_SPEED1 )
	this:turn( this.turret, y_axis, 0, TURN_SPEED1 )
	this:turn( this.cradle, x_axis, 0, TURN_SPEED1 )
	this:wait_for_turn( this.turret, y_axis )
	this:wait_for_turn( this.cradle, x_axis )
	
	this:move( this.lift, y_axis, 0, MOVE_SPEED1 )
	this:move( this.turret, y_axis, 0, MOVE_SPEED1 )
	this:sleep( SLEEP_TIME2 )

	this:turn( this.door1, z_axis, 0, TURN_SPEED1 )
	this:turn( this.door2, z_axis, 0, TURN_SPEED1 )
end

__this.ACTIVATECMD     = __this.Go
__this.DEACTIVATECMD   = __this.Stop

__this.StartBuilding = function(this, heading, pitch)
    this.buildheading = heading * TA2DEG
    this.buildpitch = pitch * TA2DEG
    this:start_script( this.RequestState, this, ACTIVE )
end

__this.StopBuilding = function(this)
    this:start_script( this.RequestState, this, INACTIVE )
end

__this.QueryNanoPiece = function(this)
	return this.firepoint
end

__this.Killed = function(this, severity)
	if severity >= 0 and severity < 25 then
		this:explode( this.nano, BITMAPONLY + BITMAP )
		this:explode( this.cradle, BITMAPONLY + BITMAP )
		this:explode( this.body, BITMAPONLY + BITMAP )
		this:explode( this.turret, BITMAPONLY + BITMAP )
		return 1
	elseif severity >= 25 and severity < 50 then
		this:explode( this.nano, FALL + BITMAP )
		this:explode( this.cradle, FALL + BITMAP )
		this:explode( this.body, BITMAPONLY + BITMAP )
		this:explode( this.turret, SHATTER + BITMAP )
		return 2
	elseif severity >= 50 and severity < 100 then
		this:explode( this.nano, SHATTER + BITMAP )
		this:explode( this.rear, SHATTER + BITMAP )
		this:explode( this.cradle, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.panel_t, FALL + BITMAP )
		this:explode( this.panel_b, FALL + BITMAP )
		this:explode( this.panel_l, FALL + BITMAP )
		this:explode( this.panel_r, FALL + BITMAP )
		this:explode( this.body, BITMAPONLY + BITMAP )
		this:explode( this.turret, SHATTER + BITMAP )
		return 3
	-- D-Gunned/Self-D
	elseif severity >= 100 then
		this:explode( this.nano, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.rear, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.cradle, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.panel_t, FALL + BITMAP )
		this:explode( this.panel_b, FALL + BITMAP )
		this:explode( this.panel_l, FALL + BITMAP )
		this:explode( this.panel_r, FALL + BITMAP )
		this:explode( this.body, SHATTER + BITMAP )
		this:explode( this.turret, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		return 3
	end
	return 0
end

__this.Create = function(this)
	this.moving = false
    this.buildheading = 0
    this.buildpitch = 0
    this.build_monitor = false

	this:hide( this.tracks1 )
	this:hide( this.tracks2 )
	this:hide( this.tracks3 )

	while this:get(BUILD_PERCENT_LEFT) > 0 do
		this:sleep( 0.25 )
	end
	
    this:InitState()
	
	this:start_script( this.AnimationControl, this )
    this:start_script( this.SmokeUnit, this )
end
