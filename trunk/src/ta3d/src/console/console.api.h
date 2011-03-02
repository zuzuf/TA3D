/*  TA3D, a remake of Total Annihilation
	Copyright (C); 2005  Roland BROCHARD

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option); any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/
#ifndef _TA3D_XX_CLASSE_CONSOLE_API_H__
# define _TA3D_XX_CLASSE_CONSOLE_API_H__

// We want to use part of game script API
#include <scripts/script.h>

namespace TA3D
{
	class CAPI
	{
		friend class Console;
	private:
		static int print(lua_State *L);
		static int setFps(lua_State *L);
		static int zshoot(lua_State *L);
		static int setFarsight(lua_State *L);
		static int setTooltips(lua_State *L);
		static int setSoundVolume(lua_State *L);
		static int setMusicVolume(lua_State *L);
		static int setRightClickInterface(lua_State *L);
		static int setCameraOrtho(lua_State *L);
		static int setGrabInputs(lua_State *L);
		static int setVideoShoot(lua_State *L);
		static int screenshot(lua_State *L);
		static int makeposter(lua_State *L);
		static int reloadShaders(lua_State *L);
		static int setShowMissionInfo(lua_State *L);
		static int setViewDebug(lua_State *L);
		static int setAIDebug(lua_State *L);
		static int setInternalName(lua_State *L);
		static int setInternalIdx(lua_State *L);
		static int exit(lua_State *L);
		static int setWireframe(lua_State *L);
		static int setPriority(lua_State *L);
		static int setWaterQuality(lua_State *L);
		static int setShadowQuality(lua_State *L);
		static int setShadowMapSize(lua_State *L);
		static int setDetailsTexture(lua_State *L);
		static int setParticles(lua_State *L);
		static int setExplosionParticles(lua_State *L);
		static int setWaves(lua_State *L);
		static int scriptDumpDebugInfo(lua_State *L);
		static int setShowModel(lua_State *L);
		static int setRotateLight(lua_State *L);
		static int shake(lua_State *L);
		static int setFreecam(lua_State *L);
		static int setFpsLimit(lua_State *L);
		static int spawn(lua_State *L);
		static int setTimeFactor(lua_State *L);
		static int addhp(lua_State *L);
		static int deactivate(lua_State *L);
		static int activate(lua_State *L);
		static int resetScript(lua_State *L);
		static int dumpUnitInfo(lua_State *L);
		static int kill(lua_State *L);
		static int setShootall(lua_State *L);
		static int startScript(lua_State *L);					// Force l'éxecution d'un script
		static int setPause(lua_State *L);
		static int give(lua_State *L);							// Give resources to player_id
		static int setMetalCheat(lua_State *L);
		static int setEnergyCheat(lua_State *L);
		static int setGUIalpha(lua_State *L);
		static int setShowPing(lua_State *L);
		// ---------------    Debug commands    ---------------
		static int _debugSetContext(lua_State *L);
		static int _debugState(lua_State *L);
		static int _debugLoad(lua_State *L);
		static int _debugStop(lua_State* L);
		static int _debugResume(lua_State *L);
		static int _debugKill(lua_State* L);
		static int _debugCrash(lua_State* L);
		static int _debugContinue(lua_State *L);
		static int _debugRun(lua_State *L);
		static int _debugMemory(lua_State *L);
		// ---------------    OS specific commands    ---------------
		static int setFullscreen(lua_State* L);
	};
}

#endif
