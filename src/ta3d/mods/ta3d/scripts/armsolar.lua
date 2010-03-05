-- Arm Solar Plant

createUnitScript("armsolar")

armsolar:piece("base","dish1","dish2","dish3","dish4")

armsolar.SIG_HIT     = 2
armsolar.SMOKEPIECE1 = armsolar.base

#include "StateChg.lh"
#include "smokeunit.lh"
#include "exptype.lh"

__this.Go = function (this)
    this:turn( this.dish1, x_axis, -90, 60 )
    this:turn( this.dish2, x_axis, 90, 60 )
    this:turn( this.dish3, z_axis, -90, 60 )
    this:turn( this.dish4, z_axis, 90, 60 )
    this:wait_for_turn( this.dish1, x_axis )
    this:wait_for_turn( this.dish2, x_axis )
    this:wait_for_turn( this.dish3, z_axis )
    this:wait_for_turn( this.dish4, z_axis )
    this:set( ARMORED, false )
end

__this.Stop = function (this)
    this:set( ARMORED, true )
    this:turn( this.dish1, x_axis, 0, 120 )
    this:turn( this.dish2, x_axis, 0, 120 )
    this:turn( this.dish3, z_axis, 0, 120 )
    this:turn( this.dish4, z_axis, 0, 120 )
    this:wait_for_turn( this.dish1, x_axis )
    this:wait_for_turn( this.dish2, x_axis )
    this:wait_for_turn( this.dish3, z_axis )
    this:wait_for_turn( this.dish4, z_axis )
end

__this.ACTIVATECMD = __this.Go
__this.DEACTIVATECMD = __this.Stop

__this.Create = function (this)
    this:InitState()
    this:start_script( this.SmokeUnit, this )
end

__this.Activate = function (this)
    this:start_script( this.RequestState, this, ACTIVE )
end

__this.Deactivate = function (this)
    this:start_script( this.RequestState, this, INACTIVE )
end

__this.HitByWeapon = function(this, anglex, anglez)
    this:signal( SIG_HIT )
    set_signal_mask( SIG_HIT )
    this:set( ACTIVATION, 0 )
    sleep( 8 )
    this:set( ACTIVATION, 1 )
end

__this.SweetSpot = function (this)
    return this.base
end

__this.Killed = function ( this, severity )
    if severity <= 25 then
        this:explode( this.dish1,    BITMAPONLY + BITMAP1 )
        this:explode( this.dish2,    BITMAPONLY + BITMAP2 )
        this:explode( this.dish3,    BITMAPONLY + BITMAP3 )
        this:explode( this.dish4,    BITMAPONLY + BITMAP4 )
        this:explode( this.base,    BITMAPONLY + BITMAP5 )
        return 1
    end

    if severity <= 50 then
        this:explode( this.dish1,    FALL + BITMAP1 )
        this:explode( this.dish2,    FALL + BITMAP2 )
        this:explode( this.dish3,    FALL + BITMAP3 )
        this:explode( this.dish4,    FALL + BITMAP4 )
        this:explode( this.base,    BITMAPONLY + BITMAP5 )
        return 2
    end

    if severity <= 99 then
        this:explode( this.dish1,    FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
        this:explode( this.dish2,    FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
        this:explode( this.dish3,    FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
        this:explode( this.dish4,    FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4 )
        this:explode( this.base,    BITMAPONLY + BITMAP5 )
        return 3
    end

    this:explode( this.dish1,    FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
    this:explode( this.dish2,    FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
    this:explode( this.dish3,    FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
    this:explode( this.dish4,    FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4 )
    this:explode( this.base,    SHATTER + EXPLODE_ON_HIT + BITMAP5 )
    return 3
end

