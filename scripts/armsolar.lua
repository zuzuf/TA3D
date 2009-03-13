-- Arm Solar Plant

print("running armsolar.lua")

piece("base","dish1","dish2","dish3","dish4")

SIG_HIT = 2
SMOKEPIECE1 = base

#include "StateChg.h"
#include "smokeunit.h"
#include "exptype.h"

function Go()
    print("Go()")
    turn_object( dish1, x_axis, -90, 60 )
    turn_object( dish2, x_axis, 90, 60 )
    turn_object( dish3, z_axis, -90, 60 )
    turn_object( dish4, z_axis, 90, 60 )
    wait_for_turn( dish1, x_axis )
    wait_for_turn( dish2, x_axis )
    wait_for_turn( dish3, z_axis )
    wait_for_turn( dish4, z_axis )
    set( ARMORED, false )
end

function Stop()
    print("Stop()")
    set( ARMORED, true )
    turn_object( dish1, x_axis, 0, 120 )
    turn_object( dish2, x_axis, 0, 120 )
    turn_object( dish3, z_axis, 0, 120 )
    turn_object( dish4, z_axis, 0, 120 )
    wait_for_turn( dish1, x_axis )
    wait_for_turn( dish2, x_axis )
    wait_for_turn( dish3, z_axis )
    wait_for_turn( dish4, z_axis )
end

ACTIVATECMD = Go
DEACTIVATECMD = Stop

function Create()
    InitState()
    start_script( SmokeUnit )
end

function Activate()
    print("Activate()")
    start_script( RequestState, ACTIVE )
end

function Deactivate()
    print("Deactivate()")
    start_script( RequestState, INACTIVE )
end

function HitByWeapon(anglex, anglez)
    signal( SIG_HIT )
    set_signal_mask( SIG_HIT )
    set( ACTIVATION, 0 )
    sleep( 8 )
    set( ACTIVATION, 1 )
end

function SweetSpot()
    return base
end

function Killed( severity, corpsetype )
    if severity <= 25 then
        corpsetype = 1
        explode( dish1,    BITMAPONLY + BITMAP1 )
        explode( dish2,    BITMAPONLY + BITMAP2 )
        explode( dish3,    BITMAPONLY + BITMAP3 )
        explode( dish4,    BITMAPONLY + BITMAP4 )
        explode( base,    BITMAPONLY + BITMAP5 )
        return 0
    end

    if severity <= 50 then
        corpsetype = 2
        explode( dish1,    FALL + BITMAP1 )
        explode( dish2,    FALL + BITMAP2 )
        explode( dish3,    FALL + BITMAP3 )
        explode( dish4,    FALL + BITMAP4 )
        explode( base,    BITMAPONLY + BITMAP5 )
        return 0
    end

    if severity <= 99 then
        corpsetype = 3
        explode( dish1,    FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
        explode( dish2,    FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
        explode( dish3,    FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
        explode( dish4,    FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4 )
        explode( base,    BITMAPONLY + BITMAP5 )
        return 0
    end

    corpsetype = 3
    explode( dish1,    FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1 )
    explode( dish2,    FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
    explode( dish3,    FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
    explode( dish4,    FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4 )
    explode( base,    SHATTER + EXPLODE_ON_HIT + BITMAP5 )
    return 0
end

