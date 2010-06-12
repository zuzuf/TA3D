createUnitScript("corstorm")

__this:piece("torso", "rbarrel2", "lbarrel2", "pelvis", "rfoot", "lfoot", "lthigh", "lexhaust", "rexhaust",
       "rthigh", "rcalf", "lcalf")

local torso = __this.torso
local pelvis = __this.pelvis
local rbarrel2 = __this.rbarrel2
local lbarrel2 = __this.lbarrel2
local rfoot = __this.rfoot
local lfoot = __this.lfoot
local lthigh = __this.lthigh
local lexhaust = __this.lexhaust
local rexhaust = __this.rexhaust
local rthigh = __this.rthigh
local rcalf = __this.rcalf
local lcalf = __this.lcalf

__this.SMOKEPIECE1 = torso

#include "sfxtype.lh"
#include "exptype.lh"
#include "smokeunit.lh"

-- Signal definitions
local SIG_MOVE = 2


__this.walk = function(this)
	while true do
		this:move_piece_now(pelvis, y_axis, 0.375)
		this:move_piece_now(rfoot, z_axis, 0.0)
		this:move_piece_now(lfoot, z_axis, 0.0)
		this:move_piece_now(torso, y_axis, 0.0)
		this:turn_piece_now(pelvis, y_axis, 0.0)
		this:turn_piece_now(torso, x_axis, -10.186813)
		this:turn_piece_now(lthigh, x_axis, 11.884615)
		this:turn_piece_now(rthigh, x_axis, 7.857143)
		this:turn_piece_now(rthigh, y_axis, 0.0)
		this:turn_piece_now(rcalf, x_axis, 0.0)
		this:turn_piece_now(rfoot, x_axis, 5.439560)
		this:turn_piece_now(lcalf, x_axis, -24.923077)
		this:turn_piece_now(lfoot, x_axis, 13.032967)
		this:sleep(0.09)

		this:move_piece_now(pelvis, y_axis, 0.15)
		this:turn_piece_now(pelvis, y_axis, 0.153846)
		this:turn_piece_now(torso, x_axis, -8.065934)
		this:turn_piece_now(lthigh, x_axis, 21.758242)
		this:turn_piece_now(rthigh, x_axis, -14.010989)
		this:turn_piece_now(rcalf, x_axis, -5.093407)
		this:turn_piece_now(rfoot, x_axis, 4.670330)
		this:turn_piece_now(lcalf, x_axis, -8.912088)
		this:turn_piece_now(lfoot, x_axis, -7.917582)
		this:sleep(0.09)

		this:move_piece_now(pelvis, y_axis, 0.075)
		this:turn_piece_now(pelvis, y_axis, -0.104396)
		this:turn_piece_now(torso, x_axis, -5.939560)
		this:turn_piece_now(lthigh, x_axis, 28.247253)
		this:turn_piece_now(rthigh, x_axis, -14.857143)
		this:turn_piece_now(rcalf, x_axis, -51.598901)
		this:turn_piece_now(rfoot, x_axis, 46.274725)
		this:turn_piece_now(lcalf, x_axis, -4.241758)
		this:turn_piece_now(lfoot, x_axis, -14.203297)
		this:sleep(0.09)

		this:move_piece_now(pelvis, y_axis, 0.0)
		this:turn_piece_now(pelvis, y_axis, -0.016484)
		this:turn_piece_now(torso, x_axis, 2.120879)
		this:turn_piece_now(lthigh, x_axis, 36.923077)
		this:turn_piece_now(rthigh, x_axis, -15.708791)
		this:turn_piece_now(rcalf, x_axis, -30.659341)
		this:turn_piece_now(rfoot, x_axis, 44.961538)
		this:turn_piece_now(lcalf, x_axis, 0.0)
		this:turn_piece_now(lfoot, x_axis, -14.010989)
		this:sleep(0.09)

		this:move_piece_now(pelvis, y_axis, 0.15)
		this:turn_piece_now(pelvis, y_axis, -0.192308)
		this:turn_piece_now(torso, x_axis, -2.120879)
		this:turn_piece_now(lthigh, x_axis, 24.521978)
		this:turn_piece_now(rthigh, x_axis, -10.741758)
		this:turn_piece_now(rcalf, x_axis, -23.054945)
		this:turn_piece_now(rfoot, x_axis, 34.500000)
		this:turn_piece_now(lcalf, x_axis, 8.439560)
		this:turn_piece_now(lfoot, x_axis, -12.840659)
		this:sleep(0.09)

		this:move_piece_now(pelvis, y_axis, 0.225)
		this:turn_piece_now(pelvis, y_axis, -0.131868)
		this:turn_piece_now(torso, x_axis, -8.065934)
		this:turn_piece_now(lthigh, x_axis, 10.038462)
		this:turn_piece_now(rthigh, x_axis, -3.956044)
		this:turn_piece_now(rcalf, x_axis, -28.708791)
		this:turn_piece_now(rfoot, x_axis, 32.741758)
		this:turn_piece_now(lcalf, x_axis, 1.054945)
		this:turn_piece_now(lfoot, x_axis, -7.214286)
		this:sleep(0.09)

		this:move_piece_now(pelvis, y_axis, 0.3)
		this:turn_piece_now(pelvis, y_axis, 0.0)
		this:turn_piece_now(torso, x_axis, -10.186813)
		this:turn_piece_now(lthigh, x_axis, -13.587912)
		this:turn_piece_now(rthigh, x_axis, 11.884615)
		this:turn_piece_now(rcalf, x_axis, -16.131868)
		this:turn_piece_now(rfoot, x_axis, 4.241758)
		this:turn_piece_now(lcalf, x_axis, 4.571429)
		this:turn_piece_now(lfoot, x_axis, -3.395604)
		this:sleep(0.09)

		this:move_piece_now(pelvis, y_axis, 0.15)
		this:turn_piece_now(pelvis, y_axis, -0.153846)
		this:turn_piece_now(torso, x_axis, -8.065934)
		this:turn_piece_now(lthigh, x_axis, -18.653846)
		this:turn_piece_now(rthigh, x_axis, 20.703297)
		this:turn_piece_now(rcalf, x_axis, -8.065934)
		this:turn_piece_now(rfoot, x_axis, -9.824176)
		this:turn_piece_now(lcalf, x_axis, -24.082418)
		this:turn_piece_now(lfoot, x_axis, 26.335165)
		this:sleep(0.09)

		this:move_piece_now(pelvis, y_axis, 0.075)
		this:turn_piece_now(pelvis, y_axis, 0.104396)
		this:turn_piece_now(torso, x_axis, -5.939560)
		this:turn_piece_now(lthigh, x_axis, -15.285714)
		this:turn_piece_now(rthigh, x_axis, 30.005495)
		this:turn_piece_now(rcalf, x_axis, -3.818681)
		this:turn_piece_now(rfoot, x_axis, -13.340659)
		this:turn_piece_now(lcalf, x_axis, -49.489011)
		this:turn_piece_now(lfoot, x_axis, 44.285714)
		this:sleep(0.09)

		this:move_piece_now(pelvis, y_axis, 0.0)
		this:turn_piece_now(pelvis, y_axis, 0.016484)
		this:turn_piece_now(torso, x_axis, 2.120879)
		this:turn_piece_now(lthigh, x_axis, -15.708791)
		this:turn_piece_now(rthigh, x_axis, 39.384615)
		this:turn_piece_now(rthigh, y_axis, 0.0)
		this:turn_piece_now(rcalf, x_axis, 0.0)
		this:turn_piece_now(rfoot, x_axis, -14.010989)
		this:turn_piece_now(lcalf, x_axis, -29.604396)
		this:turn_piece_now(lfoot, x_axis, 45.313187)
		this:sleep(0.09)

		this:move_piece_now(pelvis, y_axis, 0.3)
		this:turn_piece_now(pelvis, y_axis, -0.153846)
		this:turn_piece_now(torso, x_axis, -2.120879)
		this:turn_piece_now(lthigh, x_axis, -2.120879)
		this:turn_piece_now(rthigh, x_axis, 24.434066)
		this:turn_piece_now(rcalf, x_axis, 13.010989)
		this:turn_piece_now(rfoot, x_axis, -2.054945)
		this:turn_piece_now(lcalf, x_axis, -37.423077)
		this:turn_piece_now(lfoot, x_axis, 39.549451)
		this:sleep(0.09)

		this:move_piece_now(pelvis, y_axis, 0.4)
		this:turn_piece_now(pelvis, y_axis, 0.054945)
		this:turn_piece_now(torso, x_axis, -8.065934)
		this:turn_piece_now(lthigh, x_axis, 5.093407)
		this:turn_piece_now(rthigh, x_axis, 16.923077)
		this:turn_piece_now(rcalf, x_axis, 7.736264)
		this:turn_piece_now(rfoot, x_axis, 3.725275)
		this:turn_piece_now(lcalf, x_axis, -32.719780)
		this:turn_piece_now(lfoot, x_axis, 27.978022)
		this:sleep(0.09)
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
	this:move(pelvis, y_axis, 0.0, 0.5)
	this:turn(pelvis, y_axis, 0.0, 150.032967)
	this:turn(pelvis, z_axis, 0.0, 150.032967)
	this:turn(rthigh, x_axis, 0.0, 150.032967)
	this:turn(rcalf, x_axis, 0.0, 150.032967)
	this:turn(rfoot, x_axis, 0.0, 150.032967)
	this:turn(lthigh, x_axis, 0.0, 150.032967)
	this:turn(lcalf, x_axis, 0.0, 150.032967)
	this:turn(lfoot, x_axis, 0.0, 150.032967)
end

__this.Create = function(this)
	this.gun_1 = 0
    this.bAiming = false
    this.restoreCount = 0
	this:start_script(this.StopMoving, this)
	this:start_script(this.SmokeUnit, this)
end

__this.SweetSpot = function(this)
	return torso
end

__this.AimFromPrimary = function(this)
	return torso
end

__this.QueryPrimary = function(this)
	if this.gun_1 == 0 then
		return rbarrel2
	end
	if this.gun_1 == 1 then
		return lbarrel2
	end
end

__this.RestoreAfterDelay = function(this)
    this.restoreCount = this.restoreCount + 1
    local ncall = this.restoreCount
    this:sleep(5.0)
    if ncall ~= this.restoreCount or this.bAiming then
        return
    end
	this:turn(torso, y_axis, 0.0, 90.021978)
	this:turn(torso, x_axis, 0.0, 45.010989)
end

__this.AimPrimary = function(this, heading, pitch)
    this:set_script_value("AimPrimary", false)

    heading = heading * TA2DEG
    pitch = pitch * TA2DEG

	this:turn(torso, y_axis, heading, 90.021978)
	this:turn(torso, x_axis, -pitch, 45.010989)

    if this.bAiming then
        return
    end
    this.bAiming = true

	while this:is_turning(torso, y_axis) or this:is_turning(torso, x_axis) do
        this:yield()
    end

    this.bAiming = false

	this:start_script( this.RestoreAfterDelay, this )
    this:set_script_value("AimPrimary", true)
end

__this.FirePrimary = function(this)
	if this.gun_1 == 0 then
		this.gun_1 = 1
		this:emit_sfx( 1024, rexhaust )
		return
	end
	if this.gun_1 == 1 then
		this.gun_1 = 0
		this:emit_sfx( 1024, lexhaust )
		return
	end
end

__this.Killed = function(this, severity)
	if severity <= 25 then
		this:explode(lfoot, BITMAPONLY + BITMAP4)
		this:explode(lcalf, BITMAPONLY + BITMAP5)
		this:explode(lthigh, BITMAPONLY + BITMAP1)
		this:explode(pelvis, BITMAPONLY + BITMAP2)
		this:explode(rfoot, BITMAPONLY + BITMAP4)
		this:explode(rcalf, BITMAPONLY + BITMAP5)
		this:explode(rthigh, BITMAPONLY + BITMAP1)
		this:explode(torso, BITMAPONLY + BITMAP2)
		return 1
	end
	if severity <= 50 then
		this:explode(lfoot, FALL + BITMAP4)
		this:explode(lcalf, FALL + BITMAP5)
		this:explode(lthigh, FALL + BITMAP1)
		this:explode(pelvis, FALL + BITMAP2)
		this:explode(rfoot, FALL + BITMAP4)
		this:explode(rcalf, FALL + BITMAP5)
		this:explode(rthigh, FALL + BITMAP1)
		this:explode(torso, FALL + BITMAP2)
		return 2
	end
	if severity <= 99 then
		this:explode(lfoot, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4)
		this:explode(lcalf, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP5)
		this:explode(lthigh, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1)
		this:explode(pelvis, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2)
		this:explode(rfoot, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4)
		this:explode(rcalf, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP5)
		this:explode(rthigh, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1)
		this:explode(torso, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2)
		return 3
	end
	this:explode(lfoot, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4)
	this:explode(lcalf, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP5)
	this:explode(lthigh, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1)
	this:explode(pelvis, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2)
	this:explode(rfoot, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP4)
	this:explode(rcalf, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP5)
	this:explode(rthigh, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1)
	this:explode(torso, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP2)
    return 3
end
