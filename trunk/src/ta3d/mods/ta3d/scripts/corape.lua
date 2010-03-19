-- Core Attack Gunship script

createUnitScript("corape")

__this:piece( "barrel","flare", "base", "thrust1", "thrust2", "fan", "b1", "b2", "b3", "b4", "b5", "b6" )

__this.SMOKEPIECE1 = __this.base

#include "StateChg.lh"
#include "smokeunit.lh"

__this.Animate = function(this)
    while true do
        if this.active then
            this:emit_sfx( SFXTYPE_THRUST, this.thrust1 )
            this:emit_sfx( SFXTYPE_THRUST, this.thrust2 )
        end
        this:sleep(0.25)
    end
end

__this.Go = function(this)
    this:turn( this.b2, y_axis, 60, 90 )
    this:turn( this.b3, y_axis, 120, 90 )
    this:turn( this.b4, y_axis, 180, 90 )
    this:turn( this.b5, y_axis, 240, 90 )
    this:turn( this.b6, y_axis, 300, 90 )
    this:spin( this.fan, y_axis, 360, 180 )
    this.active = true
end

__this.Stop = function(this)
    this:turn( this.b2, y_axis, 0, 30 )
    this:turn( this.b3, y_axis, 0, 30 )
    this:turn( this.b4, y_axis, 0, 30 )
    this:turn( this.b5, y_axis, 0, 30 )
    this:turn( this.b6, y_axis, 0, 30 )
    this:stop_spin( this.fan, y_axis, 10 )
    this.active = false
end

__this.ACTIVATECMD = __this.Go
__this.DEACTIVATECMD = __this.Stop

__this.Create = function(this)
    this.active = false
    this:hide( this.flare )
    this:InitState()
    this:start_script( this.SmokeUnit, this )
    this:start_script( this.Animate, this )
end

__this.Activate = function(this)
    this:start_script( this.RequestState, this, ACTIVE )
end

__this.Deactivate = function(this)
    this:start_script( this.RequestState, this, INACTIVE )
end

__this.QueryPrimary = function(this)
    return this.flare
end

__this.QuerySecondary = function(this)
    return this.flare
end

__this.AimPrimary = function(this)
    this:set_script_value("AimPrimary", true)
end

__this.AimSecondary = function(this)
    this:set_script_value("AimSecondary", true)
end

__this.FirePrimary = function(this)
    this:show( this.flare )
    this:sleep( 0.15 )
    this:hide( this.flare )
end

__this.FireSecondary = function(this)
    this:show( this.flare )
    this:sleep( 0.15 )
    this:hide( this.flare )
end

__this.SweetSpot = function(this)
    return this.base
end

__this.Killed = function( this, severity )
    this:hide( this.flare )
    if severity <= 25 then
        this:explode( this.base,   BITMAPONLY + BITMAP1 )
        this:explode( this.flare, BITMAPONLY + BITMAP2 )
        this:explode( this.barrel,   BITMAPONLY + BITMAP4 )
        this:explode( this.thrust1,   BITMAPONLY + BITMAP2 )
        this:explode( this.thrust2,   BITMAPONLY + BITMAP3 )
        return 1
    elseif severity <= 50 then
        this:explode( this.base,   BITMAPONLY + BITMAP1 )
        this:explode( this.flare, FALL + BITMAP2 )
        this:explode( this.barrel,   FALL + BITMAP4 )
        this:explode( this.thrust1,   FALL + BITMAP2 )
        this:explode( this.thrust2,   FALL + BITMAP3 )
        this:explode( this.fan,  EXPLODE_ON_HIT )
        this:explode( this.b1,  EXPLODE_ON_HIT )
        this:explode( this.b2,  EXPLODE_ON_HIT )
        this:explode( this.b3,  EXPLODE_ON_HIT )
        this:explode( this.b4,  EXPLODE_ON_HIT )
        this:explode( this.b5,  EXPLODE_ON_HIT )
        this:explode( this.b6,  EXPLODE_ON_HIT )
        return 2
    elseif severity <= 99 then
        this:explode( this.base,   BITMAPONLY + BITMAP1 )
        this:explode( this.flare, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
        this:explode( this.barrel,   FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4 )
        this:explode( this.thrust1,   FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
        this:explode( this.thrust2,   FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
        this:explode( this.fan,  EXPLODE_ON_HIT )
        this:explode( this.b1,  EXPLODE_ON_HIT )
        this:explode( this.b2,  EXPLODE_ON_HIT )
        this:explode( this.b3,  EXPLODE_ON_HIT )
        this:explode( this.b4,  EXPLODE_ON_HIT )
        this:explode( this.b5,  EXPLODE_ON_HIT )
        this:explode( this.b6,  EXPLODE_ON_HIT )
        return 3
    end
    this:explode( this.base,   BITMAPONLY + BITMAP1 )
    this:explode( this.flare, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
    this:explode( this.barrel,   FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4 )
    this:explode( this.thrust1,   FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
    this:explode( this.thrust2,   FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3 )
    this:explode( this.fan,  EXPLODE_ON_HIT )
    this:explode( this.b1,  EXPLODE_ON_HIT )
    this:explode( this.b2,  EXPLODE_ON_HIT )
    this:explode( this.b3,  EXPLODE_ON_HIT )
    this:explode( this.b4,  EXPLODE_ON_HIT )
    this:explode( this.b5,  EXPLODE_ON_HIT )
    this:explode( this.b6,  EXPLODE_ON_HIT )
    return 3
end
