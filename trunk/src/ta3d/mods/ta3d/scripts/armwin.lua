-- Arm Wind Generator

createUnitScript("armwin")

__this:piece( "base", "fan", "cradle" )

__this.SMOKEPIECE1 = __this.base

#include "sfxtype.lh"
#include "exptype.lh"
#include "smokeunit.lh"

__this.SetSpeed = function(this, wind_speed)
    if this:get(BUILD_PERCENT_LEFT) > 0 then
        return
    end
	this:spin( this.fan, z_axis, -wind_speed * TA2DEG, 1.0 )
end

__this.SetDirection = function(this, wind_dir)
    if this:get(BUILD_PERCENT_LEFT) > 0 then
        return
    end
   this:turn( this.cradle, y_axis, wind_dir * TA2DEG - this.base_dir, 20.0 )
end

__this.Create = function(this)
    this.base_dir = math.random(0, 360)
	this:turn_piece_now( this.base, y_axis, this.base_dir )
	this:start_script( this.SmokeUnit, this )
end

__this.SweetSpot = function(this)
	return this.base
end

__this.Killed = function(this, severity)
	if severity <= 25 then
		this:explode( this.base, BITMAPONLY + BITMAP1 )
		this:explode( this.fan, BITMAPONLY + BITMAP2 )
		this:explode( this.cradle, BITMAPONLY + BITMAP3 )
		return 1
	elseif severity <= 50 then
		this:explode( this.base, BITMAPONLY + BITMAP1 )
		this:explode( this.fan, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
		this:explode( this.cradle, BITMAPONLY + BITMAP3 )
        return 2
	elseif severity <= 99 then
		this:explode( this.base, BITMAPONLY + BITMAP1 )
		this:explode( this.fan, SHATTER + BITMAP2 )
		this:explode( this.cradle, BITMAPONLY + BITMAP3 )
        return 3
	end
	this:explode( this.base, BITMAPONLY + BITMAP1 )
	this:explode( this.fan, SHATTER + BITMAP2 )
	this:explode( this.cradle, BITMAPONLY + BITMAP3 )
    return 3
end
