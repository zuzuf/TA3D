-- Core Aircraft Plant

createUnitScript("corap")

--pieces
__this:piece( "base", "bay", "narm1", "nano1", "emit1", "arm1b", "arm1", "arm1top", "arm1bot", "pow1", "pow2", "plate", "nano2", "emit2", "door1", "door2", "radar", "padbase", "pad1", "pad2", "pad3", "build")

--local vars
__this.nanoPieces = {__this.emit1, __this.emit2}
__this.nanoIdx = 1
__this.SMOKEPIECE1 = __this.bay
__this.SMOKEPIECE2 = __this.pad1
__this.SMOKEPIECE3 = __this.radar

#include "exptype.lh"
#include "StateChg.lh"
#include "smokeunit.lh"
#include "yard.lh"

--opening animation
__this.Open = function (this)
	this:signal(2)              --kill the closing animation if it is in process
	this:set_signal_mask(1)     --set the signal to kill the opening animation

	this:move(this.bay, x_axis, 9, 5.5)
	this:wait_for_move(this.bay, x_axis)

	this:turn(this.narm1, x_axis, 1.85 * RAD2DEG, 0.75 * RAD2DEG)
	this:turn(this.nano1, x_axis,  -1.309 * RAD2DEG, 0.35 * RAD2DEG)
	this:turn(this.door1, y_axis, -0.611 * RAD2DEG, 0.35 * RAD2DEG)
	this:sleep(0.6)
	this:move(this.arm1b, x_axis, 5.5, 2.5)
	this:turn(this.pow1, z_axis, -1.571 * RAD2DEG, 0.8 * RAD2DEG)
	
	this:wait_for_turn(this.door1, y_axis)

	this:turn(this.door2, y_axis, 1.571 * RAD2DEG, 0.9 * RAD2DEG)
	this:turn(this.arm1, x_axis, 1.3 * RAD2DEG, 0.35 * RAD2DEG)
	this:turn(this.arm1top, x_axis, -0.873 * RAD2DEG, 0.4 * RAD2DEG)
	this:turn(this.arm1bot, x_axis, 1.31 * RAD2DEG, 0.5 * RAD2DEG)
	this:sleep(0.2)
	this:turn(this.pow2, x_axis, 1.571 * RAD2DEG, 1 * RAD2DEG)
	
	this:wait_for_turn(this.door2, y_axis)

	this:turn(this.narm1, x_axis, 1.466 * RAD2DEG, 0.15 * RAD2DEG)
	this:move(this.plate, z_axis, 4.235, 1.6)
	this:turn(this.door1, y_axis, 0, 0.15 * RAD2DEG)
	this:turn(this.nano2, y_axis, 0.698 * RAD2DEG, 0.35 * RAD2DEG)
	this:sleep(0.5)
	this:turn(this.arm1, x_axis, 0.524 * RAD2DEG, 0.2 * RAD2DEG)

	
	this:wait_for_move(this.plate, z_axis)

	this:sleep( 0.15 )

    this:spin( this.build, y_axis, 30, 10 )
    this:spin( this.padbase, y_axis, 30, 10 )

    this:OpenYard();
    this:set( INBUILDSTANCE, true )
end

--closing animation of the factory
__this.Close = function (this)
	this:signal(1)              --kill the opening animation if it is in process
	this:set_signal_mask(2)     --set the signal to kill the closing animation

    this:CloseYard();
    this:set( INBUILDSTANCE, false )

    this:stop_spin( this.build, y_axis, 10 )
    this:stop_spin( this.padbase, y_axis, 10 )

	this:turn(this.narm1, x_axis, 1.85 * RAD2DEG, 0.25 * RAD2DEG)
	this:sleep(0.8)
	this:turn(this.arm1, x_axis, 1 * RAD2DEG, 0.45 * RAD2DEG)
	this:move(this.plate, z_axis, 0, 1.6)
	this:turn(this.door1, y_axis, -0.611 * RAD2DEG, 0.15 * RAD2DEG)
	this:turn(this.nano2, y_axis, 0, 0.35 * RAD2DEG)

	this:wait_for_move(this.plate, z_axis)

	this:turn(this.arm1top, x_axis, 0, 0.4 * RAD2DEG)
	this:turn(this.arm1bot, x_axis, 0, 0.5 * RAD2DEG)
	this:turn(this.pow2, x_axis, 0, 1 * RAD2DEG)
	this:sleep(0.2)
	this:turn(this.arm1, x_axis, 0, 0.3 * RAD2DEG)
	this:turn(this.door2, y_axis, 0, 0.9 * RAD2DEG)

	this:wait_for_turn(this.door2, y_axis)

	this:move(this.arm1b, x_axis, 0, 2.5)
	this:turn(this.pow1, z_axis, 0, 1 * RAD2DEG)
	this:sleep(0.6)
	this:turn(this.narm1, x_axis, 0, 0.75 * RAD2DEG)
	this:turn(this.nano1, x_axis,  0, 0.35 * RAD2DEG)
	this:turn(this.door1, y_axis, 0, 0.35 * RAD2DEG)
	
	this:wait_for_turn(this.door1, y_axis)

	this:move(this.bay, x_axis, 0, 5.5)
	this:wait_for_move(this.bay, x_axis)
end

__this.padchange = function (this)
	while true do
      this:sleep(1.2)
      this:hide(this.pad1)
      this:show(this.pad2)
      this:sleep(1.2)
      this:hide(this.pad2)
      this:show(this.pad3)
      this:sleep(1.2)
      this:hide(this.pad3)
      this:show(this.pad2)
      this:sleep(1.2)
      this:hide(this.pad2)
      this:show(this.pad1)
	end
end

__this.Create = function (this)
	this:start_script(this.SmokeUnit, this)
    while this:get(BUILD_PERCENT_LEFT) > 0.0 do
        this:sleep( 1.0 )
    end
	this:spin(this.radar, y_axis, 0.8 * RAD2DEG)
	this:start_script(this.padchange, this)

end

__this.QueryBuildInfo = function (this)
	return this.build
end

__this.QueryNanoPiece = function (this)
	this.nanoIdx = (this.nanoIdx % 2) + 1

	return this.nanoPieces[this.nanoIdx]
end

__this.Activate = function (this)
	this:start_script( this.Open, this )    --animation needs its own thread because Sleep and WaitForTurn will not work otherwise
end

__this.Deactivate = function (this)
	this:start_script( this.Close, this )
end

--death and wrecks
__this.Killed = function(this, severity)
	if (severity <= 25) then
		this:explode(this.pad1, EXPLODE)

		this:explode(this.radar, EXPLODE_ON_HIT)
		this:explode(this.nano1, EXPLODE_ON_HIT)
		this:explode(this.nano2, EXPLODE_ON_HIT)

		this:explode(this.door1, EXPLODE_ON_HIT)
		this:explode(this.door2, EXPLODE_ON_HIT)

		return 1 -- corpsetype

	elseif (severity <= 50) then
		this:explode(this.base, SHATTER)

		this:explode(this.pad1, EXPLODE)

		this:explode(this.radar, EXPLODE_ON_HIT)
		this:explode(this.nano1, EXPLODE_ON_HIT)
		this:explode(this.nano2, EXPLODE_ON_HIT)

		this:explode(this.door1, EXPLODE_ON_HIT)
		this:explode(this.door2, SFX.EXPLODE_ON_HIT)

		return 2 -- corpsetype
	else
		this:explode(this.base, SHATTER)
		this:explode(this.bay, SHATTER)
		this:explode(this.door1, SHATTER)
		this:explode(this.door2, SHATTER)
		this:explode(this.radar, SHATTER)

		this:explode(this.pad1, EXPLODE)

		this:explode(this.nano1, EXPLODE_ON_HIT)
		this:explode(this.nano2, EXPLODE_ON_HIT)

		return 3 -- corpsetype
	end
end
