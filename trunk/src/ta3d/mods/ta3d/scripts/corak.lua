createUnitScript("corak")

__this:piece( "torso", "rshoulder", "lshoulder", "rgun", "lgun", "pelvis", "rcalf", "rfoot", "lcalf", "lfoot", "lthigh", "rthigh", "rfire", "lfire")

local torso = __this.torso
local rshoulder = __this.rshoulder
local lshoulder = __this.lshoulder
local rgun = __this.rgun
local lgun = __this.lgun
local pelvis = __this.pelvis
local rcalf = __this.rcalf
local rfoot = __this.rfoot
local lcalf = __this.lcalf
local lfoot = __this.lfoot
local lthigh = __this.lthigh
local rthigh = __this.rthigh
local rfire = __this.rfire
local lfire = __this.lfire

__this.SMOKEPIECE1 = torso

#include "sfxtype.lh"
#include "exptype.lh"
#include "smokeunit.lh"

__this.walk = function(this)
	if this.bMoving then
		this:move_piece_now(pelvis, y_axis, -0.225)
		this:move_piece_now(rcalf, y_axis, 0.0)
		this:move_piece_now(rcalf, z_axis, 0.0)
		this:move_piece_now(rfoot, z_axis, 0.0)
		this:move_piece_now(lcalf, y_axis, 0.0)
		this:move_piece_now(lcalf, z_axis, 0.0)
		this:move_piece_now(lfoot, z_axis, 0.0)
		this:move_piece_now(lshoulder, x_axis, 0.0)
		this:move_piece_now(lshoulder, y_axis, 0.0)
		this:move_piece_now(lshoulder, z_axis, 0.0)
		this:move_piece_now(rshoulder, x_axis, 0.0)
		this:move_piece_now(rshoulder, y_axis, 0.0)
		this:move_piece_now(rshoulder, z_axis, 0.0)
		this:move_piece_now(rgun, x_axis, 0.0)
		this:move_piece_now(lgun, x_axis, 0.0)
		this:turn_piece_now(pelvis, x_axis, -0.423077)
		this:turn_piece_now(torso, x_axis, 12.648352)
		this:turn_piece_now(lthigh, x_axis, 11.060440)
		this:turn_piece_now(rthigh, x_axis, -42.060440)
		this:turn_piece_now(rcalf, x_axis, -4.857143)
		this:turn_piece_now(rfoot, x_axis, 46.934066)
		this:turn_piece_now(lcalf, x_axis, 61.000000)
		this:turn_piece_now(lfoot, x_axis, -50.390110)
		this:sleep(0.06)
	end
	if this.bMoving then
		this:move_piece_now(pelvis, y_axis, -0.2)
		this:turn_piece_now(pelvis, x_axis, -3.082418)
		this:turn_piece_now(torso, x_axis, 12.648352)
		this:turn_piece_now(lthigh, x_axis, 34.093407)
		this:turn_piece_now(rthigh, x_axis, -27.890110)
		this:turn_piece_now(rcalf, x_axis, -6.637363)
		this:turn_piece_now(rfoot, x_axis, 37.637363)
		this:turn_piece_now(lcalf, x_axis, 33.615385)
		this:turn_piece_now(lfoot, x_axis, -24.604396)
		this:sleep(0.06)
	end
	if this.bMoving then
		this:move_piece_now(pelvis, y_axis, -0.15)
		this:turn_piece_now(pelvis, x_axis, -4.857143)
		this:turn_piece_now(torso, x_axis, 12.648352)
		this:turn_piece_now(lthigh, x_axis, 28.747253)
		this:turn_piece_now(rthigh, x_axis, -19.027473)
		this:turn_piece_now(rcalf, x_axis, -4.412088)
		this:turn_piece_now(rfoot, x_axis, 27.890110)
		this:turn_piece_now(lcalf, x_axis, 13.725275)
		this:turn_piece_now(lfoot, x_axis, -25.978022)
		this:sleep(0.06)
	end
	if this.bMoving then
		this:move_piece_now(pelvis, y_axis, -0.05)
		this:turn_piece_now(pelvis, x_axis, -6.181319)
		this:turn_piece_now(torso, x_axis, 12.648352)
		this:turn_piece_now(lthigh, x_axis, 12.813187)
		this:turn_piece_now(rthigh, x_axis, 0.0)
		this:turn_piece_now(rcalf, x_axis, 3.082418)
		this:turn_piece_now(rfoot, x_axis, 2.203297)
		this:turn_piece_now(lcalf, x_axis, -12.335165)
		this:turn_piece_now(lfoot, x_axis, 7.648352)
		this:sleep(0.06)
	end
	if this.bMoving then
		this:move_piece_now(pelvis, y_axis, 0.0)
		this:turn_piece_now(pelvis, x_axis, -4.857143)
		this:turn_piece_now(torso, x_axis, 12.302198)
		this:turn_piece_now(lthigh, x_axis, -19.467033)
		this:turn_piece_now(rthigh, x_axis, 1.758242)
		this:turn_piece_now(rcalf, x_axis, 8.401099)
		this:turn_piece_now(rfoot, x_axis, -6.181319)
		this:turn_piece_now(lcalf, x_axis, -16.813187)
		this:turn_piece_now(lfoot, x_axis, 42.505495)
		this:sleep(0.06)
	end
	if this.bMoving then
		this:move_piece_now(pelvis, y_axis, -0.175)
		this:turn_piece_now(pelvis, x_axis, -3.082418)
		this:turn_piece_now(torso, x_axis, 12.302198)
		this:turn_piece_now(lthigh, x_axis, -32.324176)
		this:turn_piece_now(rthigh, x_axis, 13.280220)
		this:turn_piece_now(rcalf, x_axis, 23.159341)
		this:turn_piece_now(rfoot, x_axis, -33.170330)
		this:turn_piece_now(lfoot, x_axis, 7.357143)
		this:sleep(0.06)
	end
	if this.bMoving then
		this:move_piece_now(pelvis, y_axis, -0.2)
		this:turn_piece_now(pelvis, x_axis, -0.423077)
		this:turn_piece_now(torso, x_axis, 12.648352)
		this:turn_piece_now(lthigh, x_axis, -36.291209)
		this:turn_piece_now(rcalf, x_axis, 43.571429)
		this:turn_piece_now(rfoot, x_axis, -43.148352)
		this:turn_piece_now(lcalf, x_axis, -10.082418)
		this:turn_piece_now(lfoot, x_axis, 28.236264)
		this:sleep(0.06)
	end
	if this.bMoving then
		this:move_piece_now(pelvis, y_axis, -0.225)
		this:turn_piece_now(pelvis, x_axis, -0.423077)
		this:turn_piece_now(torso, x_axis, 12.648352)
		this:turn_piece_now(lthigh, x_axis, -42.060440)
		this:turn_piece_now(rthigh, x_axis, 12.824176)
		this:turn_piece_now(rcalf, x_axis, 60.692308)
		this:turn_piece_now(rfoot, x_axis, -44.824176)
		this:turn_piece_now(lcalf, x_axis, -4.412088)
		this:turn_piece_now(lfoot, x_axis, 46.934066)
		this:sleep(0.06)
	end
	if this.bMoving then
		this:move_piece_now(pelvis, y_axis, -0.2)
		this:turn_piece_now(pelvis, x_axis, -3.082418)
		this:turn_piece_now(torso, x_axis, 12.648352)
		this:turn_piece_now(lthigh, x_axis, -32.758242)
		this:turn_piece_now(rthigh, x_axis, 34.093407)
		this:turn_piece_now(rcalf, x_axis, 23.615385)
		this:turn_piece_now(rfoot, x_axis, -10.791209)
		this:turn_piece_now(lcalf, x_axis, -5.747253)
		this:turn_piece_now(lfoot, x_axis, 42.505495)
		this:sleep(0.06)
	end
	if this.bMoving then
		this:move_piece_now(pelvis, y_axis, -0.15)
		this:turn_piece_now(pelvis, x_axis, -4.857143)
		this:turn_piece_now(torso, x_axis, 12.648352)
		this:turn_piece_now(lthigh, x_axis, -25.225275)
		this:turn_piece_now(rthigh, x_axis, 28.769231)
		this:turn_piece_now(rcalf, x_axis, 10.527473)
		this:turn_piece_now(rfoot, x_axis, -20.978022)
		this:turn_piece_now(lcalf, x_axis, -3.978022)
		this:turn_piece_now(lfoot, x_axis, 33.648352)
		this:sleep(0.06)
	end
	if this.bMoving then
		this:move_piece_now(pelvis, y_axis, -0.45)
		this:turn_piece_now(pelvis, x_axis, -6.181319)
		this:turn_piece_now(torso, x_axis, 12.648352)
		this:turn_piece_now(lthigh, x_axis, -12.390110)
		this:turn_piece_now(rthigh, x_axis, -0.423077)
		this:turn_piece_now(rcalf, x_axis, -11.659341)
		this:turn_piece_now(rfoot, x_axis, 22.978022)
		this:turn_piece_now(lcalf, x_axis, 4.115385)
		this:turn_piece_now(lfoot, x_axis, 14.016484)
		this:sleep(0.06)
	end
	if this.bMoving then
		this:move_piece_now(pelvis, y_axis, 0.0)
		this:turn_piece_now(pelvis, x_axis, -4.857143)
		this:turn_piece_now(lthigh, x_axis, 0.0)
		this:turn_piece_now(rthigh, x_axis, -19.467033)
		this:turn_piece_now(rcalf, x_axis, -20.302198)
		this:turn_piece_now(rfoot, x_axis, 23.445055)
		this:turn_piece_now(lcalf, x_axis, 24.203297)
		this:turn_piece_now(lfoot, x_axis, -20.736264)
		this:sleep(0.06)
	end
	if this.bMoving then
		this:move_piece_now(pelvis, y_axis, -0.175)
		this:turn_piece_now(pelvis, x_axis, -3.082418)
		this:turn_piece_now(lthigh, x_axis, 9.280220)
		this:turn_piece_now(rthigh, x_axis, -28.769231)
		this:turn_piece_now(rcalf, x_axis, -25.225275)
		this:turn_piece_now(rfoot, x_axis, 26.115385)
		this:turn_piece_now(lcalf, x_axis, 31.868132)
		this:turn_piece_now(lfoot, x_axis, -37.637363)
		this:sleep(0.06)
	end
	this:move_piece_now(pelvis, y_axis, -0.2)
	this:turn_piece_now(pelvis, x_axis, -0.423077)
	this:turn_piece_now(torso, x_axis, 12.648352)
	this:turn_piece_now(lthigh, x_axis, 11.060440)
	this:turn_piece_now(rthigh, x_axis, -37.192308)
	this:turn_piece_now(rcalf, x_axis, -9.280220)
	this:turn_piece_now(rfoot, x_axis, 6.104396)
	this:turn_piece_now(lcalf, x_axis, 47.604396)
	this:turn_piece_now(lfoot, x_axis, -47.412088)
	this:sleep(0.06)
end

__this.walklegs = function(this)
	if this.bMoving then
		this:move_piece_now(pelvis, y_axis, -0.225)
		this:move_piece_now(rcalf, y_axis, 0.0)
		this:move_piece_now(rcalf, z_axis, 0.0)
		this:move_piece_now(rfoot, z_axis, 0.0)
		this:move_piece_now(lcalf, y_axis, 0.0)
		this:move_piece_now(lcalf, z_axis, 0.0)
		this:move_piece_now(lfoot, z_axis, 0.0)
		this:move_piece_now(lshoulder, x_axis, 0.0)
		this:move_piece_now(lshoulder, y_axis, 0.0)
		this:move_piece_now(lshoulder, z_axis, 0.0)
		this:move_piece_now(rshoulder, x_axis, 0.0)
		this:move_piece_now(rshoulder, y_axis, 0.0)
		this:move_piece_now(rshoulder, z_axis, 0.0)
		this:move_piece_now(rgun, x_axis, 0.0)
		this:move_piece_now(lgun, x_axis, 0.0)
		this:turn_piece_now(pelvis, x_axis, -0.423077)
		this:turn_piece_now(lthigh, x_axis, 11.060440)
		this:turn_piece_now(rthigh, x_axis, -42.060440)
		this:turn_piece_now(rcalf, x_axis, -4.857143)
		this:turn_piece_now(rfoot, x_axis, 46.934066)
		this:turn_piece_now(lcalf, x_axis, 61.000000)
		this:turn_piece_now(lfoot, x_axis, -50.390110)
		this:sleep(0.06)
	end
	if this.bMoving then
		this:move_piece_now(pelvis, y_axis, -0.2)
		this:turn_piece_now(pelvis, x_axis, -3.082418)
		this:turn_piece_now(lthigh, x_axis, 34.093407)
		this:turn_piece_now(rthigh, x_axis, -27.890110)
		this:turn_piece_now(rcalf, x_axis, -6.637363)
		this:turn_piece_now(rfoot, x_axis, 37.637363)
		this:turn_piece_now(lcalf, x_axis, 33.615385)
		this:turn_piece_now(lfoot, x_axis, -24.604396)
		this:sleep(0.06)
	end
	if this.bMoving then
		this:move_piece_now(pelvis, y_axis, -0.15)
		this:turn_piece_now(pelvis, x_axis, -4.857143)
		this:turn_piece_now(lthigh, x_axis, 28.747253)
		this:turn_piece_now(rthigh, x_axis, -19.027473)
		this:turn_piece_now(rcalf, x_axis, -4.412088)
		this:turn_piece_now(rfoot, x_axis, 27.890110)
		this:turn_piece_now(lcalf, x_axis, 13.725275)
		this:turn_piece_now(lfoot, x_axis, -25.978022)
		this:sleep(0.06)
	end
	if this.bMoving then
		this:move_piece_now(pelvis, y_axis, -0.05)
		this:turn_piece_now(pelvis, x_axis, -6.181319)
		this:turn_piece_now(lthigh, x_axis, 12.813187)
		this:turn_piece_now(rthigh, x_axis, 0.0)
		this:turn_piece_now(rcalf, x_axis, 3.082418)
		this:turn_piece_now(rfoot, x_axis, 2.203297)
		this:turn_piece_now(lcalf, x_axis, -12.335165)
		this:turn_piece_now(lfoot, x_axis, 7.648352)
		this:sleep(0.06)
	end
	if this.bMoving then
		this:move_piece_now(pelvis, y_axis, 0.0)
		this:turn_piece_now(pelvis, x_axis, -4.857143)
		this:turn_piece_now(lthigh, x_axis, -19.467033)
		this:turn_piece_now(rthigh, x_axis, 1.758242)
		this:turn_piece_now(rcalf, x_axis, 8.401099)
		this:turn_piece_now(rfoot, x_axis, -6.181319)
		this:turn_piece_now(lcalf, x_axis, -16.813187)
		this:turn_piece_now(lfoot, x_axis, 42.505495)
		this:sleep(0.06)
	end
	if this.bMoving then
		this:move_piece_now(pelvis, y_axis, -0.175)
		this:turn_piece_now(pelvis, x_axis, -3.082418)
		this:turn_piece_now(lthigh, x_axis, -32.324176)
		this:turn_piece_now(rthigh, x_axis, 13.280220)
		this:turn_piece_now(rcalf, x_axis, 23.159341)
		this:turn_piece_now(rfoot, x_axis, -33.170330)
		this:turn_piece_now(lfoot, x_axis, 7.357143)
		this:sleep(0.06)
	end
	if this.bMoving then
		this:move_piece_now(pelvis, y_axis, -0.2)
		this:turn_piece_now(pelvis, x_axis, -0.423077)
		this:turn_piece_now(lthigh, x_axis, -36.291209)
		this:turn_piece_now(rcalf, x_axis, 43.571429)
		this:turn_piece_now(rfoot, x_axis, -43.148352)
		this:turn_piece_now(lcalf, x_axis, -10.082418)
		this:turn_piece_now(lfoot, x_axis, 28.236264)
		this:sleep(0.06)
	end
	if this.bMoving then
		this:move_piece_now(pelvis, y_axis, -0.225)
		this:turn_piece_now(pelvis, x_axis, -0.423077)
		this:turn_piece_now(lthigh, x_axis, -42.060440)
		this:turn_piece_now(rthigh, x_axis, 12.824176)
		this:turn_piece_now(rcalf, x_axis, 60.692308)
		this:turn_piece_now(rfoot, x_axis, -44.824176)
		this:turn_piece_now(lcalf, x_axis, -4.412088)
		this:turn_piece_now(lfoot, x_axis, 46.934066)
		this:sleep(0.06)
	end
	if this.bMoving then
		this:move_piece_now(pelvis, y_axis, -0.2)
		this:turn_piece_now(pelvis, x_axis, -3.082418)
		this:turn_piece_now(lthigh, x_axis, -32.758242)
		this:turn_piece_now(rthigh, x_axis, 34.093407)
		this:turn_piece_now(rcalf, x_axis, 23.615385)
		this:turn_piece_now(rfoot, x_axis, -10.791209)
		this:turn_piece_now(lcalf, x_axis, -5.747253)
		this:turn_piece_now(lfoot, x_axis, 42.505495)
		this:sleep(0.06)
	end
	if this.bMoving then
		this:move_piece_now(pelvis, y_axis, -0.15)
		this:turn_piece_now(pelvis, x_axis, -4.857143)
		this:turn_piece_now(lthigh, x_axis, -25.225275)
		this:turn_piece_now(rthigh, x_axis, 28.769231)
		this:turn_piece_now(rcalf, x_axis, 10.527473)
		this:turn_piece_now(rfoot, x_axis, -20.978022)
		this:turn_piece_now(lcalf, x_axis, -3.978022)
		this:turn_piece_now(lfoot, x_axis, 33.648352)
		this:sleep(0.06)
	end
	if this.bMoving then
		this:move_piece_now(pelvis, y_axis, -0.45)
		this:turn_piece_now(pelvis, x_axis, -6.181319)
		this:turn_piece_now(lthigh, x_axis, -12.390110)
		this:turn_piece_now(rthigh, x_axis, -0.423077)
		this:turn_piece_now(rcalf, x_axis, -11.659341)
		this:turn_piece_now(rfoot, x_axis, 22.978022)
		this:turn_piece_now(lcalf, x_axis, 4.115385)
		this:turn_piece_now(lfoot, x_axis, 14.016484)
		this:sleep(0.06)
	end
	if this.bMoving then
		this:move_piece_now(pelvis, y_axis, 0.0)
		this:turn_piece_now(pelvis, x_axis, -4.857143)
		this:turn_piece_now(lthigh, x_axis, 0.0)
		this:turn_piece_now(rthigh, x_axis, -19.467033)
		this:turn_piece_now(rcalf, x_axis, -20.302198)
		this:turn_piece_now(rfoot, x_axis, 23.445055)
		this:turn_piece_now(lcalf, x_axis, 24.203297)
		this:turn_piece_now(lfoot, x_axis, -20.736264)
		this:sleep(0.06)
	end
	if this.bMoving then
		this:move_piece_now(pelvis, y_axis, -0.175)
		this:turn_piece_now(pelvis, x_axis, -3.082418)
		this:turn_piece_now(lthigh, x_axis, 9.280220)
		this:turn_piece_now(rthigh, x_axis, -28.769231)
		this:turn_piece_now(rcalf, x_axis, -25.225275)
		this:turn_piece_now(rfoot, x_axis, 26.115385)
		this:turn_piece_now(lcalf, x_axis, 31.868132)
		this:turn_piece_now(lfoot, x_axis, -37.637363)
		this:sleep(0.06)
	end
	this:move_piece_now(pelvis, y_axis, -0.2)
	this:turn_piece_now(pelvis, x_axis, -0.423077)
	this:turn_piece_now(lthigh, x_axis, 11.060440)
	this:turn_piece_now(rthigh, x_axis, -37.192308)
	this:turn_piece_now(rcalf, x_axis, -9.280220)
	this:turn_piece_now(rfoot, x_axis, 6.104396)
	this:turn_piece_now(lcalf, x_axis, 47.604396)
	this:turn_piece_now(lfoot, x_axis, -47.412088)
	this:sleep(0.06)
end

__this.MotionControl = function(this)
	local justmoved = true
	while true do
		local moving = this.bMoving
		local aiming = this.bAiming
		if moving then
			if aiming then
				this.canAim = true
				this:walklegs()
			end
			if not aiming then
				this.canAim = false
				this:walk()
			end
			justmoved = true
		end
		if not moving then
			this.canAim = true
			if justmoved then
				this:move(pelvis, y_axis, 0.0, 0.5)
				this:turn(rthigh, x_axis, 0.0, 200.0)
				this:turn(rcalf, x_axis, 0.0, 200.0)
				this:turn(rfoot, x_axis, 0.0, 200.0)
				this:turn(lthigh, x_axis, 0.0, 200.0)
				this:turn(lcalf, x_axis, 0.0, 200.0)
				this:turn(lfoot, x_axis, 0.0, 200.0)
				if not aiming then
					this:turn(torso, y_axis, 0.0, 90.0)
					this:turn(rgun, x_axis, 0.0, 200.0)
					this:turn(rshoulder, x_axis, 0.0, 200.0)
					this:turn(lgun, x_axis, 0.0, 200.0)
					this:turn(lshoulder, x_axis, 0.0, 200.0)
				end
				justmoved = false
			end
			this:sleep(0.1)
		end
	end
end

__this.Create = function(this)
	this.bMoving = false
	this.bAiming = false
    this.aimingAnim = false
	this.canAim = true
	this.gun_1 = 0
    this.restoreCount = 0
	this:start_script(this.MotionControl, this)
	this:start_script(this.SmokeUnit, this)
end

__this.StartMoving = function(this)
	this.bMoving = true
end

__this.StopMoving = function(this)
	this.bMoving = false
end

__this.SweetSpot = function(this)
	return torso
end

__this.RestoreAfterDelay = function(this)
    this.restoreCount = this.restoreCount + 1
    local ncall = this.restoreCount
    this:sleep(2.75)
    if ncall ~= this.restoreCount or this.aimingAnim then
        return
    end
	this:turn(torso, y_axis, 0.0, 90.0)
	this:turn(rshoulder, x_axis, 0.0, 45.000000)
	this:turn(lshoulder, x_axis, 0.0, 45.000000)
	this:wait_for_turn(torso, y_axis)
	this:wait_for_turn(rshoulder, x_axis)
	this:wait_for_turn(lshoulder, x_axis)
	this.bAiming = false
end

__this.AimFromPrimary = function(this)
	return torso
end

__this.QueryPrimary = function(this)
	if this.gun_1 == 0 then
		return rfire
	end
	if this.gun_1 == 1 then
		return lfire
	end
end

__this.AimPrimary = function(this, heading, pitch)
    this:set_script_value("AimPrimary", false)

    if this.bAiming and not this.canAim then
        return
    end

	this.bAiming = true
	while not this.canAim do
		this:sleep(0.005)
	end

    heading = heading * TA2DEG
    pitch = pitch * TA2DEG
	this:turn(torso, y_axis, heading, 225.0)
	this:turn(lshoulder, x_axis, -pitch, 75.0)
	this:turn(rshoulder, x_axis, -pitch, 75.0)
    if this.aimingAnim then
        return
    end
    this.aimingAnim = true
	while this:is_turning(torso, y_axis) or this:is_turning(lshoulder, x_axis) or this:is_turning(rshoulder, x_axis) do
        this:yield()
    end

    this.aimingAnim = false
	this:start_script( this.RestoreAfterDelay, this)
    this:set_script_value("AimPrimary", true)
end

__this.FirePrimary = function(this)
	if this.gun_1 == 0 then
		this:show(rfire)
		this:sleep(0.1)
		this:hide(rfire)
		this.gun_1 = 1
		return
	end
	if this.gun_1 == 1 then
		this:show(lfire)
		this:sleep(0.1)
		this:hide(lfire)
		this.gun_1 = 0
		return
	end
end

__this.Killed = function(this, severity)
	if severity <= 25 then
		this:explode(lcalf, BITMAPONLY + BITMAP5)
		this:explode(lgun, BITMAPONLY + BITMAP1)
		this:explode(lthigh, BITMAPONLY + BITMAP2)
		this:explode(lshoulder, BITMAPONLY + BITMAP3)
		this:explode(pelvis, BITMAPONLY + BITMAP4)
		this:explode(rcalf, BITMAPONLY + BITMAP2)
		this:explode(rgun, BITMAPONLY + BITMAP3)
		this:explode(rthigh, BITMAPONLY + BITMAP4)
		this:explode(rshoulder, BITMAPONLY + BITMAP5)
		this:explode(torso, BITMAPONLY + BITMAP1)
		return 1
	end
	if severity <= 50 then
		this:explode(lgun, FALL + BITMAP1)
		this:explode(lshoulder, FALL + BITMAP3)
		this:explode(pelvis, FALL + BITMAP4)
		this:explode(rgun, FALL + BITMAP3)
		this:explode(rshoulder, FALL + BITMAP5)
		this:explode(torso, SHATTER + BITMAP1)
		return 2
	end
	if severity <= 99 then
		this:explode(lgun, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1)
		this:explode(lthigh, FALL + SMOKE + FIRE + BITMAP2)
		this:explode(lshoulder, FALL + SMOKE + FIRE + BITMAP3)
		this:explode(pelvis, FALL + SMOKE + FIRE + BITMAP4)
		this:explode(rgun, FALL + SMOKE + FIRE + BITMAP3)
		this:explode(rthigh, FALL + SMOKE + FIRE + BITMAP4)
		this:explode(rshoulder, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP5)
		this:explode(torso, SHATTER + BITMAP1)
		return 3
	end
	this:explode(lgun, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP1)
	this:explode(lthigh, FALL + SMOKE + FIRE + BITMAP2)
	this:explode(lshoulder, FALL + SMOKE + FIRE + BITMAP3)
	this:explode(pelvis, FALL + SMOKE + FIRE + BITMAP4)
	this:explode(rgun, FALL + SMOKE + FIRE + BITMAP3)
	this:explode(rthigh, FALL + SMOKE + FIRE + BITMAP4)
	this:explode(rshoulder, FALL + SMOKE + FIRE + EXPLODE_ON_HIT + BITMAP5)
	this:explode(torso, SHATTER + BITMAP1)
    return 3
end
