createUnitScript("corcrash")

__this:piece( "firept1", "firept2", "turret", "gunbase", "pelvis", "lleg", "rleg", "gun1",
       "gun2", "ground")

local firept1 = __this.firept1
local firept2 = __this.firept2
local turret = __this.turret
local gunbase = __this.gunbase
local pelvis = __this.pelvis
local lleg = __this.lleg
local rleg = __this.rleg
local gun1 = __this.gun1
local gun2 = __this.gun2
local ground = __this.ground

__this.SMOKEPIECE1 = torso

#include "sfxtype.lh"
#include "exptype.lh"
#include "smokeunit.lh"

-- Signal definitions
local SIG_MOVE = 2


__this.walk = function(this)
	while true do
		this:move_piece_now(pelvis, y_axis, 0.0)
		this:move_piece_now(lleg, y_axis, 0.0)
		this:move_piece_now(lleg, z_axis, 0.875)
		this:move_piece_now(rleg, y_axis, 0.0)
		this:move_piece_now(rleg, z_axis, -0.8)
		this:turn_piece_now(pelvis, x_axis, 6.0)
		this:turn_piece_now(lleg, x_axis, -6.0)
		this:turn_piece_now(rleg, x_axis, 0.0)
		this:sleep(0.06)

		this:move_piece_now(pelvis, y_axis, -0.25)
		this:move_piece_now(lleg, y_axis, 0.25)
		this:move_piece_now(rleg, y_axis, 0.25)
		this:turn_piece_now(pelvis, x_axis, 4.917582)
		this:turn_piece_now(lleg, x_axis, -4.917582)
		this:turn_piece_now(rleg, x_axis, 1.049451)
		this:sleep(0.03)

		this:move_piece_now(pelvis, y_axis, -0.15)
		this:move_piece_now(lleg, y_axis, 0.15)
		this:move_piece_now(rleg, y_axis, 0.15)
		this:turn_piece_now(pelvis, x_axis, 2.098901)
		this:turn_piece_now(lleg, x_axis, -2.098901)
		this:turn_piece_now(rleg, x_axis, 2.807692)
		this:sleep(0.02)

		this:move_piece_now(pelvis, y_axis, -0.05)
		this:move_piece_now(lleg, y_axis, 0.05)
		this:move_piece_now(rleg, y_axis, 0.05)
		this:turn_piece_now(pelvis, x_axis, 1.049451)
		this:turn_piece_now(lleg, x_axis, -1.038462)
		this:turn_piece_now(rleg, x_axis, 3.159341)
		this:sleep(0.01)

		this:move_piece_now(pelvis, y_axis, 0.0)
		this:move_piece_now(lleg, y_axis, 0.0)
		this:move_piece_now(rleg, y_axis, 0.55)
		this:move_piece_now(rleg, z_axis, -0.925)
		this:turn_piece_now(pelvis, x_axis, 0.0)
		this:turn_piece_now(lleg, x_axis, 0.0)
		this:turn_piece_now(rleg, x_axis, 0.0)
		this:sleep(0.06)

		this:move_piece_now(lleg, z_axis, 0.5)
		this:move_piece_now(rleg, z_axis, -0.175)
		this:sleep(0.06)

		this:move_piece_now(lleg, y_axis, 0.0)
		this:move_piece_now(lleg, z_axis, 0.0)
		this:move_piece_now(rleg, z_axis, 0.0)
		this:turn_piece_now(pelvis, x_axis, 0.0)
		this:turn_piece_now(lleg, x_axis, 0.0)
		this:turn_piece_now(rleg, x_axis, 0.0)
		this:sleep(0.06)

		this:move_piece_now(lleg, z_axis, -0.3)
		this:move_piece_now(rleg, z_axis, 0.425)
		this:turn_piece_now(rleg, x_axis, -9.137363)
		this:sleep(0.1)

		this:move_piece_now(lleg, z_axis, -0.6)
		this:move_piece_now(rleg, y_axis, 0.0)
		this:turn_piece_now(pelvis, x_axis, 6.000000)
		this:turn_piece_now(rleg, x_axis, -6.000000)
		this:sleep(0.06)

		this:move_piece_now(pelvis, y_axis, -0.244998)
		this:move_piece_now(lleg, y_axis, 0.25)
		this:move_piece_now(rleg, y_axis, 0.25)
		this:turn_piece_now(pelvis, x_axis, 4.565934)
		this:turn_piece_now(lleg, x_axis, 0.697802)
		this:turn_piece_now(rleg, x_axis, -4.917582)
		this:sleep(0.03)

		this:move_piece_now(pelvis, y_axis, -0.1399995)
		this:move_piece_now(lleg, y_axis, 0.15)
		this:move_piece_now(rleg, y_axis, 0.15)
		this:turn_piece_now(pelvis, x_axis, 2.098901)
		this:turn_piece_now(lleg, x_axis, 2.807692)
		this:turn_piece_now(rleg, x_axis, -2.098901)
		this:sleep(0.02)

		this:move_piece_now(pelvis, y_axis, -0.05)
		this:move_piece_now(lleg, y_axis, 0.05)
		this:move_piece_now(rleg, y_axis, 0.05)
		this:turn_piece_now(pelvis, x_axis, 1.049451)
		this:turn_piece_now(lleg, x_axis, 2.456044)
		this:turn_piece_now(rleg, x_axis, -1.038462)
		this:sleep(0.01)

		this:move_piece_now(lleg, y_axis, 0.55)
		this:move_piece_now(lleg, z_axis, -0.775)
		this:move_piece_now(rleg, y_axis, 0.25)
		this:turn_piece_now(pelvis, x_axis, 0.0)
		this:turn_piece_now(lleg, x_axis, 0.0)
		this:turn_piece_now(rleg, x_axis, 0.0)
		this:sleep(0.06)

		this:move_piece_now(lleg, z_axis, -0.35)
		this:move_piece_now(rleg, z_axis, 0.1)
		this:sleep(0.06)

		this:move_piece_now(lleg, z_axis, 0.225)
		this:move_piece_now(rleg, y_axis, 0.0)
		this:move_piece_now(rleg, z_axis, -0.15)
		this:turn_piece_now(rleg, x_axis, 0.0)
		this:sleep(0.06)

		this:move_piece_now(lleg, z_axis, 0.5)
		this:move_piece_now(rleg, y_axis, 0.0)
		this:move_piece_now(rleg, z_axis, -0.65)
		this:turn_piece_now(lleg, x_axis, -10.192308)
		this:turn_piece_now(rleg, x_axis, 0.0)
		this:sleep(0.1)
	end
end

__this.StartMoving = function(this)
	this:signal(SIG_MOVE)
	this:set_signal_mask(SIG_MOVE)
	this:walk()
end

__this.StopMoving = function(this)
	this:signal(SIG_MOVE)
	this:set_signal_mask(SIG_MOVE)
end

__this.Create = function(this)
	this.gun_3 = 0
	this.restore_delay = 3.0
    this.bAiming = false
    this.restoreCount = 0
	this:start_script( this.StopMoving, this )
	this:start_script( this.SmokeUnit, this )
end

__this.SetMaxReloadTime = function(this, t)
	this.restore_delay = t * 0.005
end

__this.SweetSpot = function(this)
	return pelvis
end

__this.AimFromPrimary = function(this)
	return turret
end

__this.QueryPrimary = function(this)
	if this.gun_3 == 0 then
		return firept1
	end
	if this.gun_3 == 1 then
		return firept2
	end
end

__this.RestoreAfterDelay = function(this)
    this.restoreCount = this.restoreCount + 1
    local ncall = this.restoreCount
    this:sleep(this.restore_delay)
    if ncall ~= this.restoreCount or this.bAiming then
        return
    end
	this:turn(turret, y_axis, 0.0, 90.0)
	this:turn(gunbase, x_axis, 0.0, 90.0)
end

__this.AimPrimary = function(this, heading, pitch)
    this:set_script_value("AimPrimary", false)
    heading = heading * TA2DEG
    pitch = pitch * TA2DEG

	this:turn(turret, y_axis, heading, 225.0)
	this:turn(gunbase, x_axis, -pitch, 45.010989)

    if this.bAiming then
        return
    end
    this.bAiming = true

	while this:is_turning(turret, y_axis) or this:is_turning(gunbase, x_axis) do
        this:yield()
    end

    this.bAiming = false
	this:start_script(this.RestoreAfterDelay, this)
    this:set_script_value("AimPrimary", true)
end

__this.FirePrimary = function(this)
	if this.gun_3 == 0 then
		this.gun_3 = 1
		this:move(gun1, z_axis, -0.75, 12.5)
		this:wait_for_move(gun1, z_axis)
		this:move(gun1, z_axis, 0.0, 1.5)
		return
	end
	if this.gun_3 == 1 then
		this.gun_3 = 0
		this:move(gun2, z_axis, -0.75, 12.5)
		this:wait_for_move(gun2, z_axis)
		this:move(gun2, z_axis, 0.0, 1.5)
	end
end

__this.Killed = function(this, severity)
	if severity <= 25 then
		this:explode(firept1, BITMAPONLY + BITMAP1)
		this:explode(firept2, BITMAPONLY + BITMAP2)
		this:explode(ground, BITMAPONLY + BITMAP3)
		this:explode(gun1, BITMAPONLY + BITMAP4)
		this:explode(gun2, BITMAPONLY + BITMAP5)
		this:explode(gunbase, BITMAPONLY + BITMAP1)
		this:explode(lleg, BITMAPONLY + BITMAP2)
		this:explode(pelvis, BITMAPONLY + BITMAP3)
		this:explode(rleg, BITMAPONLY + BITMAP4)
		this:explode(turret, BITMAPONLY + BITMAP5)
		return 1
	end
	if severity <= 50 then
		this:explode(firept1, FALL + BITMAP1)
		this:explode(firept2, FALL + BITMAP2)
		this:explode(ground, FALL + BITMAP3)
		this:explode(gun1, FALL + BITMAP4)
		this:explode(gun2, FALL + BITMAP5)
		this:explode(gunbase, BITMAPONLY + BITMAP1)
		this:explode(lleg, FALL + BITMAP2)
		this:explode(pelvis, BITMAPONLY + BITMAP3)
		this:explode(rleg, FALL + BITMAP4)
		this:explode(turret, BITMAPONLY + BITMAP5)
		return 2
	end
	if severity <= 99 then
		this:explode(firept1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1)
		this:explode(firept2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2)
		this:explode(ground, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3)
		this:explode(gun1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4)
		this:explode(gun2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP5)
		this:explode(gunbase, BITMAPONLY + BITMAP1)
		this:explode(lleg, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2)
		this:explode(pelvis, BITMAPONLY + BITMAP3)
		this:explode(rleg, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4)
		this:explode(turret, BITMAPONLY + BITMAP5)
		return 3
	end
	this:explode(firept1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1)
	this:explode(firept2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2)
	this:explode(ground, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP3)
	this:explode(gun1, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4)
	this:explode(gun2, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP5)
	this:explode(gunbase, SHATTER + EXPLODE_ON_HIT + BITMAP1)
	this:explode(lleg, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2)
	this:explode(pelvis, BITMAPONLY + BITMAP3)
	this:explode(rleg, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4)
	this:explode(turret, BITMAPONLY + BITMAP5)
    return 3
end
