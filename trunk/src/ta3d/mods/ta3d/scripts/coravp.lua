-- Arm Advanced Vehicle Plant Script

createUnitScript("coravp")

__this:piece( "base", "pipes", "blink_1", "blink_2",
              "wing_1", "bay_1", "arm_1", "nano_1", "pow_1",
              "wing_2", "bay_2", "arm_2", "nano_2", "pow_2",
              "wing_3", "bay_3", "arm_3", "nano_3", "pow_3" )

__this.SMOKEPIECE1 = __this.base
__this.spray = 0

#include "StateChg.lh"
#include "smokeunit.lh"
#include "exptype.lh"
#include "yard.lh"

__this.activatescr = function(this)
    this:turn( this.wing_1, z_axis, 90, 45)
    this:turn( this.bay_1, z_axis, -90, 45)
    this:turn( this.wing_2, x_axis, -90, 45)
    this:turn( this.bay_2, x_axis, 90, 45)
    this:turn( this.wing_3, z_axis, -90, 45)
    this:turn( this.bay_3, z_axis, 90, 45)

    this:wait_for_turn( this.bay_1, z_axis )

    this:turn( this.arm_1, z_axis, -135, 45)
    this:turn( this.nano_1, z_axis, 90, 45)
    this:turn( this.arm_2, x_axis, 135, 45)
    this:turn( this.nano_2, x_axis, -90, 45)
    this:turn( this.arm_3, z_axis, 135, 45)
    this:turn( this.nano_3, z_axis, -90, 45)

    this:wait_for_turn( this.arm_1, z_axis )

    this:turn( this.pow_1, z_axis, -90, 45)
    this:turn( this.pow_2, x_axis, 90, 45)
    this:turn( this.pow_3, z_axis, 90, 45)

    this:wait_for_turn( this.pow_1, z_axis )
end

__this.deactivatescr = function(this)
    this:turn( this.pow_1, z_axis, 0, 45)
    this:turn( this.pow_2, x_axis, 0, 45)
    this:turn( this.pow_3, z_axis, 0, 45)

    this:wait_for_turn( this.pow_1, z_axis )

    this:turn( this.arm_1, z_axis, 0, 45)
    this:turn( this.nano_1, z_axis, 0, 45)
    this:turn( this.arm_2, x_axis, 0, 45)
    this:turn( this.nano_2, x_axis, 0, 45)
    this:turn( this.arm_3, z_axis, 0, 45)
    this:turn( this.nano_3, z_axis, 0, 45)

    this:wait_for_turn( this.arm_1, z_axis )

    this:turn( this.wing_1, z_axis, 0, 45)
    this:turn( this.bay_1, z_axis, 0, 45)
    this:turn( this.wing_2, x_axis, 0, 45)
    this:turn( this.bay_2, x_axis, 0, 45)
    this:turn( this.wing_3, z_axis, 0, 45)
    this:turn( this.bay_3, z_axis, 0, 45)

    this:wait_for_turn( this.bay_1, z_axis )
end

__this.Go = function(this)
    this:activatescr()
    this:OpenYard()
    this:set( INBUILDSTANCE, true )
end

__this.Stop = function(this)
    this:sleep( 5.0 )
    this:set( INBUILDSTANCE, false )
    this:CloseYard()
    this:deactivatescr()
end

__this.ACTIVATECMD = __this.Go
__this.DEACTIVATECMD = __this.Stop

__this.Create = function(this)
    this:InitState()
    this:start_script( this.SmokeUnit, this )
end

__this.QueryNanoPiece = function(this)
    if this.spray == 0 then
        this.spray = 1
        return this.nano_1
    elseif this.spray == 1 then
        this.spray = 2
        return this.nano_2
    end
    this.spray = 0
    return this.nano_3
end

__this.Activate = function(this)
    this:start_script( this.RequestState, this, ACTIVE )
end

__this.Deactivate = function(this)
    this:start_script( this.RequestState, this, INACTIVE )
end

__this.QueryBuildInfo = function(this)
    return this.base
end

__this.SweetSpot = function(this)
    return this.base
end

__this.Killed = function( this, severity )
    if severity <= 25 then
        this:explode( this.base,   BITMAPONLY + BITMAP1 )
        this:explode( this.nano_1,  BITMAPONLY + BITMAP2 )
        this:explode( this.nano_2,  BITMAPONLY + BITMAP3 )
        this:explode( this.nano_3,  BITMAPONLY + BITMAP3 )
        this:explode( this.blink_1,  BITMAPONLY + BITMAP4 )
        this:explode( this.blink_2,  BITMAPONLY + BITMAP4 )
        this:explode( this.arm_1,    BITMAPONLY + BITMAP5 )
        this:explode( this.arm_2,    BITMAPONLY + BITMAP1 )
        this:explode( this.arm_3,    BITMAPONLY + BITMAP1 )
        this:explode( this.pow_1,   BITMAPONLY + BITMAP2 )
        this:explode( this.pow_2,   BITMAPONLY + BITMAP2 )
        this:explode( this.pow_3,   BITMAPONLY + BITMAP2 )
        this:explode( this.wing_1,  BITMAPONLY + BITMAP5 )
        this:explode( this.wing_2,  BITMAPONLY + BITMAP1 )
        this:explode( this.wing_3,  BITMAPONLY + BITMAP2 )

        return 1
    elseif severity <= 50 then
        this:explode( this.base,   BITMAPONLY + BITMAP1 )
        this:explode( this.nano_1,  FALL + BITMAP2 )
        this:explode( this.nano_2,  FALL + BITMAP3 )
        this:explode( this.nano_3,  FALL + BITMAP3 )
        this:explode( this.blink_1,  SHATTER + BITMAP4 )
        this:explode( this.blink_2,  SHATTER + BITMAP4 )
        this:explode( this.arm_1,    FALL + BITMAP5 )
        this:explode( this.arm_2,    FALL + BITMAP1 )
        this:explode( this.arm_3,    FALL + BITMAP1 )
        this:explode( this.pow_1,   FALL + BITMAP2 )
        this:explode( this.pow_2,   FALL + BITMAP3 )
        this:explode( this.pow_3,   FALL + BITMAP3 )
        this:explode( this.wing_1,  BITMAPONLY + BITMAP5 )
        this:explode( this.wing_2,  BITMAPONLY + BITMAP1 )
        this:explode( this.wing_3,  BITMAPONLY + BITMAP2 )
        return 2
    elseif severity <= 99 then
        this:explode( this.base,   BITMAPONLY + BITMAP1 )
        this:explode( this.nano_1,  FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
        this:explode( this.nano_2,  FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
        this:explode( this.blink_1,  SHATTER + BITMAP4 )
        this:explode( this.blink_2,  SHATTER + BITMAP4 )
        this:explode( this.arm_1,    FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP5 )
        this:explode( this.arm_2,    FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
        this:explode( this.arm_3,    FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
        this:explode( this.pow_1,   FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
        this:explode( this.pow_2,   FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
        this:explode( this.pow_3,   FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
        this:explode( this.wing_1,  BITMAPONLY + BITMAP5 )
        this:explode( this.wing_2,  BITMAPONLY + BITMAP1 )
        this:explode( this.wing_3,  BITMAPONLY + BITMAP2 )
        return 3
    end
    this:explode( this.base,   BITMAPONLY + BITMAP1 )
    this:explode( this.nano_1,  FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
    this:explode( this.nano_2,  FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
    this:explode( this.nano_3,  FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
    this:explode( this.blink_1,  SHATTER + EXPLODE_ON_HIT + BITMAP4 )
    this:explode( this.blink_2,  SHATTER + EXPLODE_ON_HIT + BITMAP4 )
    this:explode( this.arm_1,    FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP5 )
    this:explode( this.arm_2,    FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
    this:explode( this.arm_3,    FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP5 )
    this:explode( this.pow_1,   FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
    this:explode( this.pow_2,   FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
    this:explode( this.pow_3,    BITMAPONLY + BITMAP4 )
    this:explode( this.wing_1,  BITMAPONLY + BITMAP5 )
    this:explode( this.wing_2,  BITMAPONLY + BITMAP1 )
    this:explode( this.wing_3,  BITMAPONLY + BITMAP2 )
    return 3
end
