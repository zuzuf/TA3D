-- Arm Commander KBOT Script

createUnitScript("armcom")
__this:piece("torso","ruparm","luparm","rbigflash","nanospray","pelvis","lfirept","rthigh","rleg","lthigh","lleg")

-- State variables
__this.bMoving = false
__this.bAiming = false
__this.bCanAim = false
__this.aimtype = 0

#include "exptype.lh"

-- define signals
__this.SIG_AIM = 2

#include "walk.lh"
#include "walklegs.lh"

-- Aiming definitions

__this.AIM_NONE = 0
__this.AIM_DGUN = 1

__this.MotionControl = function(this)
	local just_moved

	-- So the stand will get reset
	just_moved = true

	while true do
		local moving = this.bMoving
		local aiming = this.bAiming

		if moving then
			if aiming then
				this.bCanAim = true
				this:walklegs()
			else

				this.bCanAim = false
				this:walk()
			end

			just_moved = true

		else
			bCanAim = true

			if just_moved then
				this:move( this.pelvis, y_axis, 0, 1)

				this:turn( this.rthigh, x_axis, 0, 200)
				this:turn( this.rleg, x_axis, 0, 200)

				this:turn( this.lthigh, x_axis, 0, 200)
				this:turn( this.lleg, x_axis, 0, 200)

				if not aiming then
					this:turn( this.torso, x_axis, 0, 90)
				end

				just_moved = false
			end
			this:sleep( 0.1 )
		end
	end
end

__this.Create = function(this)
	this:hide( this.rbigflash )
	this:hide( this.lfirept )
	this:hide( this.nanospray )

	-- Initial State
	this.bMoving = false
	this.bAiming = false
	this.bCanAim = true

	this.aimtype = this.AIM_NONE

	-- Motion control system
	this:start_script( this.MotionControl, this )
end

__this.StartMoving = function(this)
	this.bMoving = true
end

__this.StopMoving = function(this)
	this.bMoving = false
end

__this.SweetSpot = function(this)
	return this.torso
end

__this.QueryNanoPiece = function(this)
	return this.nanospray
end

__this.RestorePosition = function(this)
	this.aimtype = this.AIM_NONE;

	this:turn( this.torso, y_axis, 0, 90 )
	this:turn( this.ruparm, x_axis, 0, 45 )
	this:turn( this.luparm, x_axis, 0, 45 )

	this:wait_for_turn( this.torso, y_axis )
	this:wait_for_turn( this.ruparm, x_axis )
	this:wait_for_turn( this.luparm, x_axis )

	this.bAiming = false
end

__this.AimFromPrimary = function(this)
	return this.torso
end

__this.QueryPrimary = function(this)
	return this.lfirept
end

__this.AimPrimary = function(this, heading, pitch)
	this:set_script_value("AimPrimary", false)

	-- Don't override the big gun
	if this.aimtype == this.AIM_DGUN then
	  return
	end

	heading = heading * TA2DEG
	pitch = pitch * TA2DEG

	this:signal( this.SIG_AIM )				-- kill off other aim scripts
	this:set_signal_mask( this.SIG_AIM )	-- so other scripts can kill us

	-- Announce that we would like to aim, and wait until we can
	this.bAiming = true
	while not this.bCanAim do
		this:sleep( 0.1 )
	end

	-- Aim
	this:turn( this.torso, y_axis, heading, 300 )
	this:turn( this.luparm, x_axis, -pitch - 30, 45 )
	this:wait_for_turn( this.torso, y_axis )
	this:wait_for_turn( this.luparm, x_axis )

	this:set_script_value("AimPrimary", true)
end

__this.FirePrimary = function(this)
	-- Muzzle flare
	this:show( this.lfirept )
	this:sleep( 0.1 )
	this:hide( this.lfirept )
end

__this.AimFromTertiary = function(this)
	return this.torso
end

__this.QueryTertiary = function(this)
	return this.rbigflash
end

__this.AimTertiary = function(this, heading, pitch)
	this:signal( this.SIG_AIM )				-- kill off other aim scripts
	this:set_signal_mask( this.SIG_AIM )	-- so other scripts can kill us

	this:set_script_value("AimTertiary", false)

	-- Announce that we would like to aim, and wait until we can
	this.aimtype = this.AIM_DGUN
	this.bAiming = true
	while not this.bCanAim do
		this:sleep( 0.1 )
	end

	heading = heading * TA2DEG
	pitch = pitch * TA2DEG

	-- Aim
	this:turn( this.torso, y_axis, heading, 300 )
	this:turn( this.ruparm, x_axis, - pitch - 24, 45 )
	this:wait_for_turn( this.torso, y_axis )
	this:wait_for_turn( this.ruparm, x_axis )

	this:set_script_value("AimTertiary", true)
end

__this.FireTertiary = function(this)
	-- Muzzle flare
	this:show( this.rbigflash )
	this:sleep( 0.1 )
	this:hide( this.rbigflash )
end

__this.StartBuilding = function(this, heading, pitch)
	-- Announce that we would like to aim, and wait until we can
	this.bAiming = true
	while not this.bCanAim do
		this:sleep( 0.1 )
	end

	heading = heading * TA2DEG
	pitch = pitch * TA2DEG

	-- Aim
	this:turn( this.torso, y_axis, heading, 300 )
	this:turn( this.luparm, x_axis, -pitch - 30, 45 )
	this:wait_for_turn( this.torso, y_axis )
	this:wait_for_turn( this.luparm, x_axis )

	-- Announce that we are ready
 	this:set( INBUILDSTANCE, true )
end

__this.TargetCleared = function(this, which)
	this:signal( this.SIG_AIM )
	this:set_signal_mask( this.SIG_AIM )
	this:RestorePosition()
end

__this.StopBuilding = function(this)
	-- We are no longer in a position to build
	this:set( INBUILDSTANCE, false )

	this:signal( this.SIG_AIM )
	this:set_signal_mask( this.SIG_AIM )
	this:RestorePosition()
end
