-- CORSENT

createUnitScript("corsent")

__this:piece( "turret", "guns", "main", "barrel1", "flare1", "barrel2", "flare2", "tracks1", "tracks2", "tracks3", "tracks4")

local TRACK_PERIOD = 0.05

__this.SMOKEPIECE1 = __this.main

#include "exptype.lh"
#include "smokeunit.lh"
#include "hitweap.lh"

__this.TrackControl = function(this) 
    while this.isMoving do
        this.tracks = this.tracks + 1
        if this.tracks == 2 then
            this:hide( this.tracks1 )
            this:show( this.tracks2 )
        elseif this.tracks == 3 then
            this:hide( this.tracks2 )
            this:show( this.tracks3 )
        elseif this.tracks == 4 then
            this:hide( this.tracks3 )
            this:show( this.tracks4 )
        else
            this.tracks = 1
            this:hide( this.tracks4 )
            this:show( this.tracks1 )
        end
        this:sleep( TRACK_PERIOD )
    end
end

__this.RockUnit = function(this, anglex, anglez)
    anglex = anglex * TA2DEG
    anglez = anglez * TA2DEG
    this:turn( this.main, x_axis, anglex, 50.0 )
    this:turn( this.main, z_axis, anglez, 50.0 )
    this:wait_for_turn( this.main, z_axis )
    this:wait_for_turn( this.main, x_axis )
    this:turn( this.main, z_axis, 0.0, 20.0 )
    this:turn( this.main, x_axis, 0.0, 20.0 )
end

__this.Create = function(this)
    this.restoreCount = 0
    this.aiming = false
    this.base = this.main
    this:hide( this.flare1 )
    this:hide( this.flare2 )
    this.restore_delay = 5.0
    this:start_script( this.SmokeUnit, this )
    this.gun = 0
    this.isMoving = false
    this.tracks = 1
end

__this.StartMoving = function(this) 
    this.isMoving = true
    this:start_script( this.TrackControl, this )
end

__this.StopMoving = function(this) 
    this.isMoving = false
end

__this.SetMaxReloadTime = function(this)
    this.restore_delay = 5.0
end

__this.RestoreAfterDelay = function(this)
    this.restoreCount = this.restoreCount + 1
    local ncall = this.restoreCount
    this:sleep( this.restore_delay )
    if ncall ~= this.restoreCount then
        return
    end
    this:turn( this.turret, y_axis, 0.0, 600.0 )
    this:turn( this.guns, x_axis, 0.0, 550.0 )
    return true
end

__this.AimPrimary = function(this, heading, pitch)
    this:set_script_value("AimPrimary", false)

    heading = heading * TA2DEG
    pitch = pitch * TA2DEG
    this:turn( this.turret, y_axis, heading, 670.0 )
    this:turn( this.guns, x_axis, -pitch, 670.0 )

    if this.aiming then
        return
    end
    this.aiming = true
    while this:is_turning( this.turret, y_axis ) or this:is_turning( this.guns, x_axis ) do
        yield()
    end
    this.aiming = false
    this:start_script( this.RestoreAfterDelay, this )
    this:set_script_value("AimPrimary", true)
    return true
end

__this.FirePrimary = function(this)
    if this.gun == 0 then
        this:move_piece_now( this.barrel1, z_axis, -2.0 )
        this:show( this.flare1 )
        this:sleep( 0.15 )
        this:hide( this.flare1 )
        this:move( this.barrel1, z_axis, 0.0, 7.5 )

    elseif this.gun == 1 then
        this:move_piece_now( this.barrel2, z_axis, -2.0 )
        this:show( this.flare2 )
        this:sleep( 0.15 )
        this:hide( this.flare2 )
        this:move( this.barrel2, z_axis, 0.0, 7.5 )
    end
    this:sleep( 0.05 )
    this.gun = (this.gun + 1) % 2
end

__this.AimFromPrimary = function(this)
    return this.turret
end

__this.QueryPrimary = function(this)
    if this.gun == 0 then
        return this.flare1
    end
        return this.flare2
end

__this.SweetSpot = function(this)
    return this.turret
end

__this.Killed = function(this, severity)
    this:hide( this.flare1 )
    this:hide( this.flare2 )
    if severity <= 25 then
        this:explode( this.main, BITMAPONLY + BITMAP2 )
        this:explode( this.barrel1, BITMAPONLY + BITMAP3 )
        this:explode( this.barrel2, BITMAPONLY + BITMAP1 )
        this:explode( this.turret, BITMAPONLY + BITMAP5 )
        return 1
    elseif severity <= 50 then
        this:explode( this.main, BITMAPONLY + BITMAP2 )
        this:explode( this.barrel1, FALL + BITMAP3 )
        this:explode( this.barrel2, FALL + BITMAP1 )
        this:explode( this.turret, SHATTER + BITMAP5 )
        return 2
    elseif severity <= 99 then
        this:explode( this.main, BITMAPONLY + BITMAP2 )
        this:explode( this.barrel1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
        this:explode( this.barrel2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
        this:explode( this.turret, SHATTER + BITMAP5 )
        return 3
    end
    this:explode( this.main, BITMAPONLY + BITMAP2 )
    this:explode( this.barrel1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
    this:explode( this.barrel2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
    this:explode( this.turret, SHATTER + BITMAP5 )
    return 3
end
