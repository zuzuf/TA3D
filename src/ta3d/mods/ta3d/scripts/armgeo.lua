-- Arm Geo Power Plant Script
createUnitScript("armgeo")

__this:piece("base", "fan", "smoke", "smoke2")

__this.SMOKEPIECE1 = __this.base

#include "exptype.lh"
#include "smokeunit.lh"

__this.SmokeEmission = function(this)
    while true do
        this:emit_sfx( SFXTYPE_WHITESMOKE, this.smoke )
        this:emit_sfx( SFXTYPE_WHITESMOKE, this.smoke2 )
        this:sleep( 0.1 )
    end
end

__this.Create = function(this)
    -- Wait until the unit is actually built
    this:turn_piece_now( this.fan, x_axis, 180 )
    this:move_piece_now( this.fan, y_axis, 2.0 )
    while this:get(BUILD_PERCENT_LEFT) > 0 do
        sleep(0.4)
    end
    this:spin( this.fan, y_axis, 90, 90 )
	this:start_script( this.SmokeUnit, this )
    this:start_script( this.SmokeEmission, this )
end

__this.SweetSpot = function(this)
	return this.base
end

__this.Killed = function( this, severity )
	if severity <= 25 then
		this:explode( this.base, BITMAPONLY + BITMAP1 )
		this:explode( this.fan, BITMAPONLY + BITMAP2 )
		return 1
	end

	if severity <= 50 then
		this:explode( this.base, BITMAPONLY + BITMAP1 )
		this:explode( this.fan, FALL + BITMAP2 )
		return 2
	end

	if severity <= 99 then
		this:explode( this.base, BITMAPONLY + BITMAP1 )
		this:explode( this.fan, FALL + BITMAP2 )
		return 3
	end

	this:explode( this.base, BITMAPONLY + BITMAP1 )
	this:explode( this.fan, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
	return 3
end

