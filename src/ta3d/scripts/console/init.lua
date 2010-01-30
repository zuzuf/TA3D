print("initializing console")

function setCameraPerspective(v)
  if v ~= nil then
	return not setCameraOrtho(not v)
  end
  return not setCameraOrtho()
end

__fn_state = {}
__fn_state["fps"] = setFps;
__fn_state["farsight"] = setFarsight;
__fn_state["rightClickInterface"] = setRightClickInterface;
__fn_state["perspective"] = setCameraPerspective;
__fn_state["grabInputs"] = setGrabInputs;
__fn_state["videoCapture"] = setVideoShoot;
__fn_state["showMissionInfo"] = setShowMissionInfo;
__fn_state["viewDebug"] = setViewDebug;
__fn_state["AIDebug"] = setAIDebug;
__fn_state["internalName"] = setInternalName;
__fn_state["internalIdx"] = setInternalIdx;
__fn_state["wireframe"] = setWireframe;
__fn_state["detailsTexture"] = setDetailsTexture;
__fn_state["particles"] = setParticles;
__fn_state["explosionParticles"] = setExplosionParticles;
__fn_state["waves"] = setWaves;
__fn_state["showModel"] = setShowModel;
__fn_state["rotateLight"] = setRotateLight;
__fn_state["freecam"] = setFreecam;
__fn_state["shootall"] = setShootall;
__fn_state["pause"] = setPause;
__fn_state["metalCheat"] = setMetalCheat;
__fn_state["energyCheat"] = setEnergyCheat;
__fn_state["fullscreen"] = setFullscreen;

toggle = {}
setmetatable( toggle, { __index =
function(table, key)
  return function() __fn_state[key](not __fn_state[key]()) end
end} )

enable = {}
setmetatable( enable, { __index =
function(table, key)
  return function() __fn_state[key](true) end
end} )

disable = {}
setmetatable( disable, { __index =
function(table, key)
  return function() __fn_state[key](false) end
end} )