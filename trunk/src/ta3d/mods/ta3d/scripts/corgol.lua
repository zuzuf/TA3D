-- CORGOL

createUnitScript("corgol")

__this:piece( "base", "body", "turret1", "sleeve1", "barrel1", "firepoint1", "turret2", "sleeve2", "barrel2", "firepoint2",
              "tracks1", "tracks2", "tracks3", "wheels1", "wheels2", "wheels3", "wheels4", "wheels5", "wheels6" )

__this.moving = false
__this.once = 0
__this.animCount = 0

-- Signal definitions
local ANIM_SPEED = 0.05
local RESTORE_DELAY = 3.0

__this.SMOKEPIECE1 = __this.body
__this.SMOKEPIECE2 = __this.turret1

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
	this:turn( this.turret1, y_axis, 0, 45 )
	this:turn( this.sleeve1, x_axis, 0, 15 )
	this:turn( this.turret2, y_axis, 0, 120 )
	this:turn( this.sleeve2, x_axis, 0, 90 )
	
end

__this.AnimationControl = function(this)
	local current_track = 0
	
	while true do
		if this.moving or this.once > 0 then
			if current_track == 0 then
				this:show( this.tracks1 )
				this:hide( this.tracks3 )
				current_track = 1
			elseif current_track == 1 then
				this:show( this.tracks2 )
				this:hide( this.tracks1 )
				current_track = 2
			elseif current_track == 2 then
				this:show( this.tracks3 )
				this:hide( this.tracks2 )
				current_track = 0
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

	this:spin( this.wheels1, x_axis, 360, 60 )
	this:spin( this.wheels6, x_axis, 360, 60 )

	this:spin( this.wheels2, x_axis, 480, 120 )
	this:spin( this.wheels3, x_axis, 480, 120 )
	this:spin( this.wheels4, x_axis, 480, 120 )
	this:spin( this.wheels5, x_axis, 480, 120 )
end

__this.StopMoving = function(this)
	this.moving = false
	
	-- I don't like insta braking. It's not perfect but works for most cases.
	-- Probably looks goofy when the unit is turtling around, i.e. does not get faster as time increases..
	this.once = this.animCount * ANIM_SPEED / 1000
	if this.once > 3 then
        this.once = 3
    end

	this:stop_spin( this.wheels1, x_axis, 15 )
	this:stop_spin( this.wheels6, x_axis, 15 )

	this:stop_spin( this.wheels2, x_axis, 45 )
	this:stop_spin( this.wheels3, x_axis, 45 )
	this:stop_spin( this.wheels4, x_axis, 45 )
	this:stop_spin( this.wheels5, x_axis, 45 )

end


-- Weapons
__this.AimFromPrimary = function(this)
	return this.turret1
end

__this.QueryPrimary = function(this)
	return this.firepoint1
end

__this.AimPrimary = function(this, heading, pitch)
    this:set_script_value("AimPrimary", false)
    
    heading = heading * TA2DEG
    pitch = pitch * TA2DEG

	this:turn( this.turret1, y_axis, heading, 90 )
	this:turn( this.sleeve1, x_axis, -pitch, 45 )

    if this.aiming1 then
        return
    end
    this.aiming1 = true

	while this:is_turning( this.turret1, y_axis ) or this:is_turning( this.sleeve1, x_axis ) do
        yield()
	end

    this.aiming1 = false
    
    this:start_script( this.RestoreAfterDelay, this, RESTORE_DELAY )
    this:set_script_value("AimPrimary", true)
end

__this.FirePrimary = function(this)
	this:move_piece_now( this.barrel1, z_axis, -2.5 )
	this:sleep( 0.125 )
	this:move( this.barrel1, z_axis, 0, 5 )
end

__this.AimFromSecondary = function(this)
	return this.turret2
end

__this.QuerySecondary = function(this)
	return this.firepoint2
end

__this.AimSecondary = function(this, heading, pitch)
    this:set_script_value("AimSecondary", false)
    
    heading = heading * TA2DEG
    pitch = pitch * TA2DEG

	this:turn( this.turret2, y_axis, heading, 360 )
	this:turn( this.sleeve2, x_axis, -pitch, 180 )
    if this.aiming2 then
        return
    end
    this.aiming2 = true
    while this:is_turning( this.turret2, y_axis ) or this:is_turning( this.sleeve2, x_axis ) do
        yield()
    end
    this.aiming2 = false

	this:start_script( this.RestoreAfterDelay, this, RESTORE_DELAY )
    this:set_script_value("AimSecondary", true)
end

__this.FireSecondary = function(this)
	this:move_piece_now( this.barrel2, z_axis, -0.25 )
	this:move( this.barrel2, z_axis, 0, 5 )
end

__this.Killed = function(this, severity)
	if severity >= 0 and severity < 25 then
		this:explode( this.barrel1, BITMAPONLY + BITMAP )
		this:explode( this.sleeve1, BITMAPONLY + BITMAP )
		this:explode( this.barrel2, BITMAPONLY + BITMAP )
		this:explode( this.turret1, BITMAPONLY + BITMAP )
		this:explode( this.turret2, BITMAPONLY + BITMAP )
		this:explode( this.body, BITMAPONLY + BITMAP )
        return 1
    elseif severity >= 25 and severity < 50 then
		this:explode( this.barrel1, FALL + BITMAP )
		this:explode( this.barrel2, SHATTER + BITMAP )
		this:explode( this.sleeve1, FALL + BITMAP )
		this:explode( this.turret1, SHATTER + BITMAP )
		this:explode( this.turret2, FALL + BITMAP )
		this:explode( this.body, BITMAPONLY + BITMAP )
        return 2
	elseif severity >= 50 and severity < 100 then
		this:explode( this.barrel1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.barrel2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.sleeve1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.turret1, SHATTER + BITMAP )
		this:explode( this.turret2, SHATTER + BITMAP )
		this:explode( this.body, BITMAPONLY + BITMAP )
        return 3
	-- D-Gunned/Self-D
	elseif severity >= 100 then
		this:explode( this.barrel1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.barrel2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.sleeve1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.turret1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.turret2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP )
		this:explode( this.body, SHATTER + BITMAP )
		return 3
	end
end

__this.Create = function(this)

	this.moving = false
    this.restoreCount = 0
    this.aiming1 = false
    this.aiming2 = false

	this:hide( this.tracks1 )
	this:hide( this.tracks2 )

	while this:get(BUILD_PERCENT_LEFT) > 0 do
		this:sleep( 0.25 )
	end

	this:start_script( this.AnimationControl, this )
    this:start_script( this.SmokeUnit, this )
end
