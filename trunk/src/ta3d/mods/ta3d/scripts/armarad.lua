-- Arm Radar Tower script

createUnitScript("armarad")
__this:piece( "base", "turret", "dish1", "dish2", "arm1", "arm2", "post" )

__this.SIG_HIT = 4
__this.SMOKEPIECE1 = __this.base

#include "statechg.lh"
#include "smokeunit.lh"
#include "exptype.lh"

__this.Go = function(this)
	this:move( this.post, y_axis, 9.1, 16.134753 )
	this:wait_for_move( this.post, y_axis )
	this:turn( this.dish1, z_axis, 84.150000, 147.631579 )
	this:turn( this.dish2, z_axis, -84.150000, 147.631579 )
	this:wait_for_turn( this.dish1, z_axis )
	this:wait_for_turn( this.dish2, z_axis )
	this:spin( this.turret, y_axis, -30 )
	this:spin( this.arm1, x_axis, 100 )
	this:spin( this.arm2, x_axis, -100 )
end

__this.Stop = function(this)
	this:turn( this.turret, y_axis, 0, 60 )
	this:turn( this.arm1, x_axis, 0, 100 )
	this:turn( this.arm2, x_axis, 0, 100 )
	this:wait_for_turn( this.turret, y_axis )
	this:wait_for_turn( this.arm1, x_axis )
	this:wait_for_turn( this.arm2, x_axis )
	this:turn( this.dish1, z_axis, 0, 178.662420 )
	this:turn( this.dish2, z_axis, 0, 178.662420 )
	this:wait_for_turn( this.dish1, z_axis )
	this:wait_for_turn( this.dish2, z_axis )
	this:move( this.post, y_axis, 0, 19.320597 )
	this:wait_for_move( this.post, y_axis )
end

__this.ACTIVATECMD = __this.Go
__this.DEACTIVATECMD = __this.Stop

__this.Create = function(this)
	this:dont_shade( this.turret )
	this:dont_shade( this.arm1 )
	this:dont_shade( this.arm2 )
	this:dont_shade( this.dish1 )
	this:dont_shade( this.dish2 )
	this:dont_cache( this.turret )
	this:dont_cache( this.arm1 )
	this:dont_cache( this.arm2 )
	this:dont_cache( this.dish1 )
	this:dont_cache( this.dish2 )
	this:InitState()
	this:start_script( this.SmokeUnit, this )
end

__this.Activate = function(this)
	this:start_script( this.RequestState, this, ACTIVE )
end

__this.Deactivate = function(this)
	this:start_script( this.RequestState, this, INACTIVE )
end

__this.HitByWeapon = function(this, anglex, anglez)
	this:signal( this.SIG_HIT )
	this:set_signal_mask( this.SIG_HIT )
	this:set( ACTIVATION, 0 )
	this:sleep( 8 )
	this:set( ACTIVATION, 1 )
end

__this.SweetSpot = function(this)
	return this.base
end

__this.Killed = function(this, severity)
	if severity <= 25 then
		this:explode( arm1, BITMAPONLY + BITMAP2 )
		this:explode( arm2, BITMAPONLY + BITMAP2 )
		this:explode( base, BITMAPONLY + BITMAP2 )
		this:explode( dish1, BITMAPONLY + BITMAP1 )
		this:explode( dish2, BITMAPONLY + BITMAP1 )
		this:explode( post, BITMAPONLY + BITMAP1 )
		this:explode( turret, BITMAPONLY + BITMAP1 )
		return 1
	end

	if severity <= 50 then
		this:explode( arm1, FALL + BITMAP2 )
		this:explode( arm2, FALL + BITMAP2 )
		this:explode( base, BITMAPONLY + BITMAP2 )
		this:explode( dish1, BITMAPONLY + BITMAP1 )
		this:explode( dish2, BITMAPONLY + BITMAP1 )
		this:explode( post, BITMAPONLY + BITMAP1 )
		this:explode( turret, SHATTER + BITMAP1 )
		return 2
	end

	if severity <= 99 then
		this:explode( arm1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
		this:explode( arm2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
		this:explode( base, BITMAPONLY + BITMAP2 )
		this:explode( dish1, BITMAPONLY + BITMAP1 )
		this:explode( dish2, BITMAPONLY + BITMAP1 )
		this:explode( post, BITMAPONLY + BITMAP1 )
		this:explode( turret, SHATTER + BITMAP1 )
		return 3
	end

	this:explode( arm1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
	this:explode( arm2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2 )
	this:explode( base, BITMAPONLY + BITMAP2 )
	this:explode( dish1, BITMAPONLY + BITMAP1 )
	this:explode( dish2, BITMAPONLY + BITMAP1 )
	this:explode( post, BITMAPONLY + BITMAP1 )
	this:explode( turret, SHATTER + EXPLODE_ON_HIT + BITMAP1 )
	return 3
end
