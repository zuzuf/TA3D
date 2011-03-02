print("Initializing console")

function setCameraPerspective(v)
  if v ~= nil then
	return not setCameraOrtho(not v)
  end
  return not setCameraOrtho()
end

__fn_state = {}
__fn_state["fps"] = setFps;
__fn_state["farsight"] = setFarsight;
__fn_state["tooltips"] = setTooltips;
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
__fn_state["showPing"] = setShowPing;

debug = {}
debug["setContext"] = _debugSetContext
debug["state"] = _debugState
debug["load"] = _debugLoad
debug["stop"] = _debugStop
debug["resume"] = _debugResume
debug["kill"] = _debugKill
debug["crash"] = _debugCrash
debug["continue"] = _debugContinue
debug["run"] = _debugRun
debug["memory"] = _debugMemory


local __tab_complete_fn = function(param)
  local result = nil
  for k,v in pairs(__fn_state) do
	k = k .. "()"
	if k:sub(1,param:len()) == param then
	  if result ~= nil then
		result = result .. ", "
	  else
		result = ""
	  end
	  result = result .. k
	end
  end
  if result == nil then
	return ""
  end
  return result
end

local __tab_complete_debug = function(param)
  local result = nil
  for k,v in pairs(debug) do
	k = k .. "()"
	if k:sub(1,param:len()) == param and k:sub(1,2) ~= "__" then
	  if result ~= nil then
		result = result .. ", "
	  else
		result = ""
	  end
	  result = result .. k
	end
  end
  if result == nil then
	return ""
  end
  return result
end

toggle = {__tab_complete = __tab_complete_fn}
setmetatable( toggle, { __index =
function(table, key)
  return function() __fn_state[key](not __fn_state[key]()) end
end} )

enable = {__tab_complete = __tab_complete_fn}
setmetatable( enable, { __index =
function(table, key)
  return function() __fn_state[key](true) end
end} )

disable = {__tab_complete = __tab_complete_fn}
setmetatable( disable, { __index =
function(table, key)
  return function() __fn_state[key](false) end
end} )

debug.__tab_complete = __tab_complete_debug

energy = function() toggle.energyCheat() end
metal = function() toggle.metalCheat() end
pause = function() toggle.pause() end

__cmd_list = {}
__cmd_list["toggle."] = true;
__cmd_list["enable."] = true;
__cmd_list["disable."] = true;
__cmd_list["debug."] = true;
__cmd_list["exit()"] = true;

for k, v in pairs(_G) do
  if type(v) == "function" then
	__cmd_list[k .. "()"] = true;
  end
end

__tab_complete = function(param)
  local result = nil
  for k,v in pairs(__cmd_list) do
	if k:sub(1,param:len()) == param then
	  if result ~= nil then
		result = result .. ", "
	  else
		result = ""
	  end
	  result = result .. k
	end
  end
  if result == nil then
	return ""
  end
  return result
end

print("Console initialized")
