/*  TA3D, a remake of Total Annihilation
	Copyright (C) 2005  Roland BROCHARD

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/
#ifndef __TA3D__INGAM__BATTLE_H__
# define __TA3D__INGAM__BATTLE_H__

# include <stdafx.h>
# include <misc/string.h>
# include "gamedata.h"
# include <memory> // auto_ptr
# include <gfx/gui/area.h>
# include <gfx/sky.h>
# include <gfx/texture.h>
# include <tdf.h>
# include <EngineClass.h>
# include <misc/rect.h>
# include <misc/material.light.h>
# include <scripts/script.h>
# include "fps.h"


# ifndef SCROLL_SPEED
#	define SCROLL_SPEED 	400.0f
# endif

namespace TA3D
{
	namespace Menus
	{
		class Loading;
	}

	class Battle
	{
		friend class CAPI;
		friend class Console;
	public:
		//! Results about the battle
		enum Result {brUnknown, brVictory, brDefeat, brPat};

		/*!
		** \brief Launch a game
		** \param g Data about the game
		** \return The result of the game
		*/
		static Result Execute(GameData* g);

	public:
		//! \name Constructors & Destructor
		//@{
		//! Constructor with a GameData
		Battle(GameData* g);
		//! Destructor
		~Battle();
		//@}

		/*!
		** \brief Launch the battle
		*/
		Result execute();

		/*!
		** \brief The result of the battle
		*/
		Result result() const {return pResult;}

		void setCameraDirection(const Vector3D &dir);
		void setFreeCamera(bool fc);

		void setShowPing(bool b)	{	bShowPing = b;	}
		bool getShowPing() const	{	return bShowPing;	}

	private:
		/*!
		** \brief Reset the cache for the GUI name
		**
		** To avoid multiple and expensive string manipulations
		*/
		void updateCurrentGUICacheNames();

		/*!
		** \brief update fog parameters
		*/
		void updateFOG();

		/*!
		** \brief update camera zfar parameter
		*/
		void updateZFAR();

		//! \name Preparing all data about a battle
		//@{
		/*!
		** \brief Load Game Data
		*/
		bool loadFromGameData(GameData* g);
		//! PreInitialization
		bool initPreflight(GameData* g);
		//! Load textures
		bool initTextures();
		//! Load 3D models
		bool init3DModels();
		//! Load graphical features
		bool initGraphicalFeatures();
		//! Load Weapons
		bool initWeapons();
		//! Load units
		bool initUnits();
		//! Intermediate clean up
		bool initIntermediateCleanup();
		//! Init the engine
		bool initEngine();
		//! Add players
		bool initPlayers();
		//! Init restrictions
		bool initRestrictions();
		//! Load the GUI
		bool initGUI();
		//! Init the map
		bool initTheMap();
		//! Init the Sky
		bool initTheSky();
		//! Init the sun
		bool initTheSun();
		//! Init all textures
		bool initAllTextures();
		//! Init the camera
		bool initTheCamera();
		//! Init the wind
		bool initTheWind();
		//! Init the fog
		bool initTheFog();
		//! Init particules
		bool initParticules();
		//! Init the Water
		bool initTheWater();
		//! Init miscellaneous stuff
		bool initPostFlight();
		//@}

		void waitForNetworkPlayers();

		//! \name Preflight
		//@{

		/*!
		** \brief Reinit vars
		*/
		void preflightVars();

		/*!
		** \brief Change the speed and the direction of the wind
		*/
		void preflightChangeWindSpeedAndDirection();

		/*!
		** \brief Update the 3D sounds
		*/
		void preflightUpdate3DSounds();

		/*!
		** \brief Replace the camera automatically
		*/
		void preflightAutomaticCamera();
		/*!
		** \brief Replace the camera
		*/
		void preflightFreeCamera();

		/*!
		** \brief
		*/
		void preflightFindCursor();

		/*!
		** \brief Pre Execute
		*/
		bool preExecute(LuaProgram& gameScript);
		//@}

		//! \name 2D Objects && User interaction
		//@{
		void draw2DObjects();
		void draw2DMouseUserSelection();
		//@}

		//! \name Renderer
		//@{
		void initRenderer();
		void renderScene();
		void renderReflection();
		void renderStencilShadow();
		void renderShadowMap();
		void renderWater();
		void renderWorld();
		void renderInfo();
		void renderPostEffects();
		void makePoster(int w, int h);
		//@}

		//! \name Evens in Game
		//@{
		//@}

		void reloadShaders();

		Vector3D cursorOnMap(const Camera& cam, MAP& map, bool on_mini_map = false);

		
		void handleGameStatusEvents();
		/*!
		** \brief Display some game informations (key `SPACE`)
		*/
		void showGameStatus();

		void keyArrowsInFreeCam();
		void keyArrowsNotInFreeCam();

	private:
		//! Results
		Result pResult;
		//! Data about the current game
		GameData* pGameData;

		//! Use network communications
		bool pNetworkEnabled;
		//! Host a game
		bool pNetworkIsServer;

        //! The game script
        LuaProgram game_script;

	private:
		enum CurrentGUICache {cgcDot, cgcShow, cgcHide, cgcEnd};
		//! The current GUI
		String pCurrentGUI;
		//!
		String pCurrentGUICache[cgcEnd];
		//!
		HashSet<>::Sparse toBeLoadedMenuSet;

	private:
		//!
		
		//!
		struct DebugInfo
		{
			//!
			LuaThread *process;
		};

	private:
		//! The area
		Gui::AREA pArea;
		//! Informations about FPS
		FPSInfos fps;
		//! The map of the game - TODO The auto_ptr is deprecated
		std::auto_ptr<MAP> map;

		//! \name Sky
		//@{
		//!
		float sky_angle;
		//!
		Sky sky;
		//@}

		bool pMouseSelecting;
		//! The bounding box of the current mouse selection (if pMouseSelecting == true)
		Rect<int> pMouseRectSelection;

		//! The sun
		HWLight pSun;

		//! \name Textures
		//@{
		//!
		Interfaces::GfxTexture	freecam_on;
		//!
		Interfaces::GfxTexture	freecam_off;
		//!
		Interfaces::GfxTexture	arrow_texture;
		//!
		Interfaces::GfxTexture	circle_texture;
		//!
		GLuint	water;
		//!
		GLuint	water_sim0;
		//!
		GLuint	water_sim1;
		//!
		GLuint	water_sim2;
		//!
		GLuint	water_distortions;
		//!
		Interfaces::GfxTexture	pause_tex;
		//@}

		//! \name Camera
		//@{
		//!
		Camera cam;
		//!
		Vector3D cam_target;
		//! The position of the camera on the virtual "rail"
		float camera_zscroll;
		//!
		int cam_target_mx;
		//!
		int cam_target_my;
		//!
		bool cam_has_target;
		//!
		bool escMenuWasVisible;
		//!
		bool freecam;
		//! Just to see if the cam has been long enough at the default angle
		int cam_def_timer;
		//! Tracking a unit ? negative value => no
		int track_mode;
		//!
		bool last_time_activated_track_mode;
		//!
		bool showHealthBars;
		//!
		float r1;
		//!
		float r2;
		//!
		float r3;
		//!
		float render_time;
		//@}

		//! \name Unknown vars
		//@{
		//!
		bool show_model;
		//!
		bool rotate_light;
		//!
		float light_angle;
		//!
		bool cheat_metal;
		//!
		bool cheat_energy;
		//!
		bool internal_name;
		//!
		bool internal_idx;
		//!
		bool ia_debug;
		//!
		bool view_dbg;
		//!
		bool show_mission_info;
		//!
		bool speed_changed;
		//!
		float show_timefactor;
		//!
		float show_gamestatus;
		//!
		int	unit_info_id;
		//!
		float speed_limit;
		//!
		float delay;
		//!
		int nb_shoot;
		//!
		bool shoot;
		//!
		bool ordered_destruct;
		//!
		bool tilde;
		//!
		bool done;
		//!
		int mx;
		//!
		int my;
		//!
		int omb;
		//!
		int omb2;
		//!
		int omb3;
		//!
		int amx;
		//!
		int amy;
		//!
		int cur_sel;
		//!
		int old_gui_sel;
		//!
		bool old_sel;
		//!
		bool selected;
		//!
		int	build; // Indique si l'utilisateur veut construire quelque chose
		//!
		bool build_order_given;
		//!
		int cur_sel_index;
		//!
		int omz;
		//!
		float cam_h;
		//!
		float dt;
		//!
		float t;
		//!
		int count;
		//!
		bool reflection_drawn_last_time;
		//!
		int video_timer;
		//!
		bool video_shoot;
		//!
		int current_order;
		//@}

		//! \name Wind
		//@{
		//!
		float wind_t;
		//!
		bool wind_change;
		//@}

		//! \name Fog
		//@{
		//!
		float FogD;
		//!
		float FogNear;
		//!
		float FogFar;
		//!
		float FogColor[4];
		//!
		GLuint FogMode;
		//@}

		//! \name Water
		//@{
		//!
		WATER::Ptr water_obj;
		//!
		Shader water_shader;
		//!
		Shader water_shader_reflec;
		//!
		Shader water_pass1;
		//!
		Shader water_pass1_low;
		//!
		Shader water_pass2;
		//!
		Shader water_simulator_shader;
		//!
		Shader water_simulator_shader2;
		//!
		Shader water_simulator_shader3;
		//!
		Shader water_simulator_shader4;
		//!
		Shader water_distortions_shader;
		//!
		Shader water_simulator_reflec;
		//!
		GLuint height_tex;
		//!
		GLuint transtex;
		//!
		GLuint reflectex;
		//!
		GLuint first_pass;
		//!
		GLuint second_pass;
		//!
		GLuint water_color;
		//!
		GLuint water_FBO;
		//!
		uint32 last_water_refresh;
		//@}

		//! \name Interface
		//@{
		//!
		bool IsOnGUI;
		//!
		bool IsOnMinimap;
		//!
		bool can_be_there;
		//!
		bool rope_selection;
		//@}

	private:
		bool pCacheShowGameStatus;

        //! \name Debug information
        DebugInfo debugInfo;

		//! pointer to loading screen
		Menus::Loading *loading;

		//! show players ping
		bool bShowPing;

	private:
		static Battle *pInstance;

	public:
		static Battle *Instance();

	}; // class Battle



} // namespace TA3D

#endif // __TA3D__INGAM__BATTLE_H__
