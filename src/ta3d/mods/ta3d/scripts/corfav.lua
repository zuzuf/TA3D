-- CORFAV

createUnitScript("corfav")

__this:piece( "base", "body", "turret", "sleeve", "barrel", "firepoint1", "firepoint2", "firepoint3", "exhaust",
			"wheel1", "wheel2", "wheel3", "wheel4", "wheel5", "wheel6", "sensor1", "sensor2", "sensor3",
			"sensor4", "sensor5", "sensor6", "sensor7", "sensor8" )

__this.SMOKEPIECE1 = __this.body
__this.SMOKEPIECE2 = __this.turret
__this.aim1 = false

local RESTORE_DELAY = 1.5

local TURRET_TURN_SPEED = 240

local WHEEL_TURN_SPEED1               = 480
local WHEEL_TURN_SPEED1_ACCELERATION  = 45
local WHEEL_TURN_SPEED1_DECELERATION  = 60

local WHEEL_TURN_SPEED2               = 360
local WHEEL_TURN_SPEED2_ACCELERATION  = 30    
local WHEEL_TURN_SPEED2_DECELERATION  = 45

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

__this.StartMoving = function(this)
	this:spin( this.wheel1, x_axis, WHEEL_TURN_SPEED2, WHEEL_TURN_SPEED2_ACCELERATION )
	this:spin( this.wheel2, x_axis, WHEEL_TURN_SPEED2, WHEEL_TURN_SPEED2_ACCELERATION )
	this:spin( this.wheel3, x_axis, WHEEL_TURN_SPEED1, WHEEL_TURN_SPEED1_ACCELERATION )
	this:spin( this.wheel4, x_axis, WHEEL_TURN_SPEED1, WHEEL_TURN_SPEED1_ACCELERATION )
	this:spin( this.wheel5, x_axis, WHEEL_TURN_SPEED1, WHEEL_TURN_SPEED1_ACCELERATION )
	this:spin( this.wheel6, x_axis, WHEEL_TURN_SPEED1, WHEEL_TURN_SPEED1_ACCELERATION )
end

__this.StopMoving = function(this)
	this:stop_spin( this.wheel1, x_axis, WHEEL_TURN_SPEED2_DECELERATION )
	this:stop_spin( this.wheel2, x_axis, WHEEL_TURN_SPEED2_DECELERATION )
	this:stop_spin( this.wheel3, x_axis, WHEEL_TURN_SPEED1_DECELERATION )
	this:stop_spin( this.wheel4, x_axis, WHEEL_TURN_SPEED1_DECELERATION )
	this:stop_spin( this.wheel5, x_axis, WHEEL_TURN_SPEED1_DECELERATION )
	this:stop_spin( this.wheel6, x_axis, WHEEL_TURN_SPEED1_DECELERATION )
end

-- Weapons
__this.AimFromPrimary = function(this)
	return this.turret
end

__this.QueryPrimary = function(this)
	return this.firepoint1
end

__this.AimPrimary = function(this, heading, pitch)
    this:set_script_value("AimPrimary", false)

    heading = heading * TA2DEG
    pitch = pitch * TA2DEG

	this:turn( this.turret, y_axis, heading, TURRET_TURN_SPEED )

    if this.aim1 then
        return
    end
    this.aim1 = true

	this:wait_for_turn( this.turret, y_axis )

    this.aim1 = false

	this:start_script( this.RestoreAfterDelay, this, RESTORE_DELAY )

    this:set_script_value("AimPrimary", true)
end

__this.FirePrimary = function(this)
	this:emit_sfx( SFXTYPE_WHITESMOKE, this.firepoint1 )
end

__this.AimFromSecondary = function(this)
	return this.body
end

__this.QuerySecondary = function(this)
	if this.gun2 then
		return this.firepoint2
	else
		return this.firepoint3
	end
end

__this.AimSecondary = function(this, heading, pitch)
    this:set_script_value("AimSecondary", true)
end

__this.FireSecondary = function(this)
	if this.gun2 then
		this.gun2 = not this.gun2
		this:emit_sfx( UNIT_SFX1, this.firepoint2 )
	else
		this.gun2 = not this.gun2
		this:emit_sfx( UNIT_SFX1, this.firepoint3 )
	end
end

__this.Killed = function(this, severity)
	if severity >= 0 and severity < 25 then
		this:explode( this.barrel, BITMAPONLY + BITMAP )
		this:explode( this.body  , BITMAPONLY + BITMAP )
		this:explode( this.turret, BITMAPONLY + BITMAP )
		this:explode( this.wheel1, BITMAPONLY + BITMAP )
		this:explode( this.wheel2, BITMAPONLY + BITMAP )
		this:explode( this.wheel3, BITMAPONLY + BITMAP )
		this:explode( this.wheel4, BITMAPONLY + BITMAP )
		this:explode( this.wheel5, BITMAPONLY + BITMAP )
		this:explode( this.wheel6, BITMAPONLY + BITMAP )
        return 1
	elseif severity >= 25 and severity < 50 then
		this:explode( this.barrel, FALL + BITMAP )
		this:explode( this.body  , BITMAPONLY + BITMAP )
		this:explode( this.turret, SHATTER + BITMAP )
		this:explode( this.wheel1, BITMAPONLY + BITMAP )
		this:explode( this.wheel2, FALL + BITMAP )
		this:explode( this.wheel3, BITMAPONLY + BITMAP )
		this:explode( this.wheel4, FALL + BITMAP )
		this:explode( this.wheel5, FALL + BITMAP )
		this:explode( this.wheel6, BITMAPONLY + BITMAP )
        return 2
	elseif severity >= 50 and severity < 100 then
		this:explode( this.barrel, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.body  , BITMAPONLY + BITMAP )
		this:explode( this.turret, SHATTER + BITMAP )
		this:explode( this.wheel1, FALL + FIRE + BITMAP )
		this:explode( this.wheel2, FALL + BITMAP )
		this:explode( this.wheel3, BITMAPONLY + BITMAP )
		this:explode( this.wheel4, SHATTER + BITMAP )
		this:explode( this.wheel5, FALL + FIRE + BITMAP )
		this:explode( this.wheel6, BITMAPONLY + BITMAP )
        return 3
	-- D-Gunned/Self-D
	elseif severity >= 100 then
		this:explode( this.barrel, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.body  , SHATTER + BITMAP )
		this:explode( this.turret, SHATTER + BITMAP )
		this:explode( this.wheel1, SHATTER + BITMAP )
		this:explode( this.wheel2, FALL + BITMAP )
		this:explode( this.wheel3, BITMAPONLY + BITMAP )
		this:explode( this.wheel4, SHATTER + BITMAP )
		this:explode( this.wheel5, FALL + FIRE + BITMAP )
		this:explode( this.wheel6, FALL + FIRE + BITMAP )
        return 3
	end
end

__this.Create = function(this)
	this.gun2 = false
    this.restoreCount = 0

	this:turn_piece_now( this.exhaust, y_axis, 180 )

    this:start_script( this.SmokeUnit, this )
end
