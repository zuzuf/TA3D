-- Arm Wind Generator

createUnitScript("corwin")

__this:piece ( "base", "blades" )

__this.SMOKEPIECE1 = __this.base

#include "sfxtype.lh"
#include "exptype.lh"
#include "smokeunit.lh"

__this.SetSpeed = function(this, wind_speed)
    if this:get(BUILD_PERCENT_LEFT) > 0 then
        return
    end
    this:spin( this.blades, y_axis, -wind_speed * TA2DEG, 1.0 )
end

__this.SweetSpot = function(this)
    return this.base
end

__this.Create = function(this)
    this:dont_cache( this.blades )
    this:dont_shade( this.blades )
    this:turn_piece_now( this.base, y_axis, this.base_dir )
    this:start_script( this.SmokeUnit, this )
end

__this.Killed = function(this, severity)
	if severity <= 25 then
		this:explode( this.base, BITMAPONLY + BITMAP1 )
		this:explode( this.blades, BITMAPONLY + BITMAP2 )
		return 1
	end
	if severity <= 50 then
		this:explode( this.base, BITMAPONLY + BITMAP1 )
		this:explode( this.blades, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
		return 2
	end
	if severity <= 99 then
		this:explode( this.base, BITMAPONLY + BITMAP1 )
		this:explode( this.blades, SHATTER + BITMAP2 )
        return 3
	end
	this:explode( this.base, BITMAPONLY + BITMAP1 )
	this:explode( this.blades, SHATTER + BITMAP2 )
    return 3
end
