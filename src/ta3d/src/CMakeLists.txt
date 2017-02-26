
STRING(STRIP "${LIBS}" LIBS)
LINK_LIBRARIES("${LIBS} ${SDL_LIBRARY}")

# Yuni Settins
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Yuni_CXXFLAGS}")
link_libraries("${Yuni_LIBS}")



include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR})

set(TA3D_FORMS)

#
# STDAFX
#
SET(SRC_STDAFX stdafx.h)

#
#
#
SET(SRC_FILEFORMATS
		gaf.cpp gaf.h)

#
# Meshs
#
SET(SRC_MESHES
		mesh/mesh.cpp mesh/mesh.h
		mesh/3do.cpp mesh/3do.h
		mesh/textures.cpp mesh/textures.h
		mesh/instancing.cpp mesh/instancing.h
		mesh/3dm.cpp mesh/3dm.h
		mesh/s3o.cpp mesh/s3o.h
		mesh/obj.cpp mesh/obj.h
		mesh/3ds.cpp mesh/3ds.h
		mesh/joins.cpp mesh/joins.h)

#
# Logs
#
SET(SRC_LOGS
                logs/logs.h logs/logs.cpp)

#
# Languages
SET(SRC_I18N
		languages/table.h languages/table.cpp
		languages/i18n.h languages/i18n.hxx languages/i18n.cpp)


#
# Threads
#
SET(SRC_THREADS
		threads/mutex.h threads/mutex.cpp
		threads/thread.h threads/thread.cpp)

#
# Misc
# TODO Must of those files should be removed/cleaned
#
SET(SRC_MISC
		misc/string.h misc/string.cpp
		misc/stack.hxx
		misc/progressnotifier.h
		misc/paths.h misc/paths.cpp
		misc/files.h misc/files.cpp
		misc/resources.h misc/resources.cpp
		misc/settings.h misc/settings.cpp
		misc/interface.h misc/interface.cpp
		misc/hash_table.h
		misc/osinfo.h misc/osinfo.cpp
		misc/application.h misc/application.cpp
		misc/point.h misc/rect.h
		misc/kdtree.h misc/kdtree.hxx
		misc/bvh.h misc/bvh.hxx
		misc/bitmap.h
		misc/mempool.h

		misc/vector.cpp misc/vector.h
		misc/matrix.h misc/matrix.cpp
		misc/material.light.h misc/material.light.cpp
		misc/camera.h misc/camera.cpp
		misc/recttest.h misc/recttest.cpp

                misc/broadcastingiodevice.h misc/broadcastingiodevice.cpp
                misc/timer.h misc/timer.cpp
		misc/grid.h misc/grid.hxx misc/grid.cpp
		misc/math.h misc/math.cpp
                misc/tdf.h misc/tdf.cpp)
		

#
# Mods
SET(SRC_MODS
		mods/mods.h mods/mods.cpp
		mods/modinfo.h mods/modinfo.cpp)


#
# Scripts
#
SET(SRC_SCRIPTS
		scripts/cob.cpp scripts/cob.h
		scripts/cob.vm.h scripts/cob.vm.cpp
		scripts/lua.thread.h scripts/lua.thread.cpp
		scripts/lua.env.h scripts/lua.env.cpp
		scripts/lua.chunk.h scripts/lua.chunk.hxx scripts/lua.chunk.cpp
		scripts/lua.data.h scripts/lua.data.cpp
		scripts/draw.list.h scripts/draw.list.cpp
		scripts/unit.script.h scripts/unit.script.cpp
		scripts/unit.script.func.cpp
		scripts/script.interface.h scripts/script.interface.cpp
		scripts/unit.script.interface.h scripts/unit.script.interface.cpp
		scripts/ai.script.h scripts/ai.script.cpp
		scripts/script.data.h scripts/script.data.cpp
		scripts/script.h scripts/script.cpp
		scripts/noscript.h scripts/noscript.cpp)

#
# GFX
#
SET(SRC_GFX
		gfx/gfx.h gfx/gfx.cpp
		gfx/gfx.toolkit.h gfx/gfx.toolkit.cpp
		gfx/font.cpp gfx/font.h
		gfx/glfunc.h gfx/glfunc.cpp
		gfx/fx.base.h gfx/fx.base.cpp
		gfx/fx.particle.h gfx/fx.particle.cpp
		gfx/fx.electric.h gfx/fx.electric.cpp
		gfx/fx.manager.h gfx/fx.manager.cpp
		gfx/fx.h
		gfx/shader.h gfx/shader.cpp
		gfx/sky.h gfx/sky.cpp
		gfx/video.h gfx/video.cpp
		gfx/gl.extensions.h
		gfx/texture.cpp gfx/texture.h)

#
# GFX - GUI
#
SET(SRC_GFX_GUI
		gfx/gui/base.h gfx/gui/base.cpp
		gfx/gui/object.h gfx/gui/object.cpp
		gfx/gui/skin.h gfx/gui/skin.cpp
		gfx/gui/skin.object.h gfx/gui/skin.object.cpp
		gfx/gui/text.color.h gfx/gui/text.color.cpp
		gfx/gui/skin.manager.h gfx/gui/skin.manager.cpp
		gfx/gui/wnd.h gfx/gui/wnd.cpp
		gfx/gui/area.h gfx/gui/area.cpp)

SET(SRC_GFX_PARTICLES
		gfx/particles/particles.h gfx/particles/particle.h
		gfx/particles/particlessystem.h gfx/particles/particlessystem.cpp
		gfx/particles/particlesengine.h gfx/particles/particlesengine.cpp
)

#
# Sound
#
SET(SRC_SOUND
		sounds/manager.h sounds/manager.cpp
		)


#
# Debug
#
SET(SRC_DEBUG
		backtrace.cpp backtrace.h
                bugreportdialog.cpp bugreportdialog.h
                configdialog.cpp configdialog.h
		config.cpp)
set(TA3D_FORMS ${TA3D_FORMS}
                bugreportdialog.ui
                configdialog.ui)

#
# Console
#
SET(SRC_CONSOLE
		console/console.api.cpp console/console.api.h
		console/console.cpp console/console.h)

#
# VFS (HPI, archives)
#
SET(SRC_VFS
                vfs/vfs.cpp vfs/vfs.h
		vfs/archive.cpp vfs/archive.h
		vfs/hpi.cpp vfs/hpi.h
                vfs/realfs.cpp vfs/realfs.h)


SET(SRC_ENGINE
		UnitEngine.cpp UnitEngine.h
		EngineClass.cpp EngineClass.h
		engine/weapondata.h  engine/weapondata.hxx
		engine/unit.h  engine/unit.cpp engine/unit.defines.h
		engine/mission.h engine/mission.hxx engine/mission.cpp
		engine.cpp engine.h
		)


SET(SRC_INPUT
		input/mouse.h input/mouse.cpp
		input/keyboard.cpp input/keyboard.h)

SET(SRC_NETWORK
		network/network.cpp network/network.h
		network/socket.cpp network/socket.h
		network/socket.tcp.cpp network/socket.tcp.h
		network/socket.broadcast.cpp network/socket.broadcast.h
		network/socket.udp.cpp network/socket.udp.h
		network/ta3dsock.cpp network/ta3dsock.h
		network/networkutils.h network/networkutils.cpp
		network/socketlist.h network/socketlist.cpp
		network/TA3D_Network.cpp network/TA3D_Network.h
		network/netclient.cpp network/netclient.h network/netclient.hxx
		network/http.cpp network/http.h)

SET(SRC_AI
		ai/ai.h ai/ai.hxx ai/ai.cpp
		ai/brain.h ai/brain.cpp
		ai/weight.h ai/weight.cpp
		ai/ai.controller.h ai/ai.controller.cpp
		ai/pathfinding.h ai/pathfinding.cpp	)


SET(SRC_INGAME
		ingame/sidedata.h ingame/sidedata.cpp
		ingame/gamedata.h ingame/gamedata.cpp
		ingame/players.h ingame/players.hxx ingame/players.cpp
		ingame/battle.h ingame/battle.init.cpp
		ingame/battle.preflight.cpp ingame/battle.cpp ingame/battle.tools.cpp ingame/battle.renderer.cpp
		ingame/fps.h ingame/fps.cpp
		)

SET(SRC_INGAME_WEAPONS
		ingame/weapons/weapons.h ingame/weapons/weapons.cpp
		ingame/weapons/weapons.def.h ingame/weapons/weapons.def.cpp
		ingame/weapons/weapons.ingame.h ingame/weapons/weapons.ingame.cpp
		ingame/weapons/weapons.manager.h ingame/weapons/weapons.manager.cpp
		ingame/weapons/weapons.single.h ingame/weapons/weapons.single.cpp)

SET(SRC_INGAME_MENUS
		ingame/menus/base.h ingame/menus/base.cpp
		ingame/menus/splash.h ingame/menus/splash.cpp
		ingame/menus/intro.h ingame/menus/intro.cpp
		ingame/menus/loading.h ingame/menus/loading.cpp
		ingame/menus/solo.h ingame/menus/solo.cpp
		ingame/menus/mapselector.h ingame/menus/mapselector.cpp
		ingame/menus/unitselector.h ingame/menus/unitselector.cpp
		ingame/menus/statistics.h ingame/menus/statistics.cpp
		ingame/menus/mainmenu.h ingame/menus/mainmenu.cpp
		ingame/menus/netmenu.h ingame/menus/netmenu.cpp
		ingame/menus/multimenu.h ingame/menus/multimenu.cpp
		ingame/menus/campaignmainmenu.h ingame/menus/campaignmainmenu.cpp
		ingame/menus/briefscreen.h ingame/menus/briefscreen.cpp
		ingame/menus/waitroom.h ingame/menus/waitroom.cpp
		ingame/menus/networkroom.h ingame/menus/networkroom.cpp
		ingame/menus/config.h ingame/menus/config.cpp
		ingame/menus/setupgame.h ingame/menus/setupgame.cpp)

SET(SRC_ZUZUF
                zuzuf/hashfn.h
                zuzuf/hashmap.h
                zuzuf/hashset.h
                zuzuf/murmurhash2.h
                zuzuf/smartptr.h)

qt5_wrap_ui(TA3D_UI ${TA3D_FORMS})

# --------------------------------------------------------------------
# --- TA3D
# --------------------------------------------------------------------
SET(TA3D_BIN ta3d)


SET(TA3D_SOURCES
		${SRC_STDAFX} ${SRC_LOGS} ${SRC_I18N} ${SRC_MISC}
		${SRC_THREADS} ${SRC_DEBUG} ${SRC_CONSOLE}
		${SRC_GFX} ${SRC_GFX_GUI}
		${SRC_GFX_PARTICLES}
		${SRC_MESHES}
		${SRC_MODS}
		${SRC_SOUND}
		${SRC_SCRIPTS}
		${SRC_ENGINE}
		${SRC_INPUT}
		${SRC_FILEFORMATS}
		${SRC_VFS}
		${SRC_NETWORK}
		${SRC_AI}
		${SRC_INGAME} ${SRC_INGAME_WEAPONS} ${SRC_INGAME_MENUS}
                ${SRC_ZUZUF}
                ${TA3D_UI}
		install.cpp
		restore.cpp restore.h
		fbi.cpp fbi.h
		tdf.cpp tdf.h tnt.cpp tnt.h
		TA3D_NameSpace.cpp TA3D_NameSpace.h ta3dbase.h
		hpiview.cpp
		cache.h cache.cpp
		main.cpp sdl.h
		)







IF(NOT APPLE)

	ADD_EXECUTABLE(${TA3D_BIN} ${TA3D_SOURCES})

ELSE(NOT APPLE)

	#
	# --- TA3D Program / OS XBundle ---
	#

	# OS X : Add a single file as a bundle resource
	MACRO(BUNDLE_INSTALL srcfolder relfolder filename)
		File(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Resources/${relfolder}/")
		ADD_CUSTOM_COMMAND(OUTPUT "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Resources/${relfolder}/${filename}"
			COMMAND cp ARGS -f "${PROJECT_SOURCE_DIR}/${srcfolder}/${filename}"
				"${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Resources/${relfolder}/${filename}"
			DEPENDS "${PROJECT_SOURCE_DIR}/${srcfolder}/${filename}")
		SET(TA3D_SOURCES ${TA3D_SOURCES} "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Resources/${relfolder}/${filename}")
	ENDMACRO(BUNDLE_INSTALL)

	MACRO(BUNDLE_INSTALL_GLOB srcfolder relfolder globfiles)
		File(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Resources/${relfolder}/")
		File(GLOB lst "${PROJECT_SOURCE_DIR}/${srcfolder}/${globfiles}")
		ForEach(fl ${lst})
			String(REGEX REPLACE "^/.*/([^/]+)$" "\\1" fl "${fl}")
			ADD_CUSTOM_COMMAND(OUTPUT "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Resources/${relfolder}/${fl}"
				COMMAND cp ARGS -f "${PROJECT_SOURCE_DIR}/${srcfolder}/${fl}"
					"${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Resources/${relfolder}/${fl}"
				DEPENDS "${PROJECT_SOURCE_DIR}/${srcfolder}/${fl}")
			SET(TA3D_SOURCES ${TA3D_SOURCES} "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Resources/${relfolder}/${fl}")
		EndForEach(fl ${lst})
	ENDMACRO(BUNDLE_INSTALL_GLOB)

	MACRO(BUNDLE_BASIC_INSTALL srcfolder relfolder filename)
		File(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/${relfolder}/")
		ADD_CUSTOM_COMMAND(OUTPUT "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/${relfolder}/${filename}"
			COMMAND cp ARGS -f
				"${PROJECT_SOURCE_DIR}/${srcfolder}/${filename}"
				"${PROJECT_SOURCE_DIR}/ta3d.app/Contents/${relfolder}/${filename}"
			DEPENDS "${PROJECT_SOURCE_DIR}/${srcfolder}/${filename}")
		SET(TA3D_SOURCES ${TA3D_SOURCES} "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/${relfolder}/${filename}")
	ENDMACRO(BUNDLE_BASIC_INSTALL)

	MACRO(BUNDLE_ADD_LIB libname)
		ADD_CUSTOM_COMMAND(OUTPUT "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Frameworks/${libname}"
			COMMAND cp ARGS -fH "/opt/local/lib/${libname}" "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Frameworks/${libname}"
			COMMAND chmod ARGS 644 "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Frameworks/${libname}"
			DEPENDS "/opt/local/lib/${libname}")
		SET(TA3D_SOURCES ${TA3D_SOURCES} "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Frameworks/${libname}")
	ENDMACRO(BUNDLE_ADD_LIB)

	MACRO(BUNDLE_RELINK src libname)
		ADD_CUSTOM_COMMAND(TARGET ${TA3D_BIN} POST_BUILD
			COMMAND install_name_tool ARGS -change '/opt/local/lib/${libname}'
				'@executable_path/../Frameworks/${libname}' "${PROJECT_SOURCE_DIR}/ta3d.app/${src}"
		)
	ENDMACRO(BUNDLE_RELINK)

	File(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Resources/")
	File(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Frameworks/")
	File(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/MacOS/")

	# Ta3d mod
	BUNDLE_BASIC_INSTALL("./" "MacOS" "ta3d.mod")
	# Misc
	BUNDLE_BASIC_INSTALL("../../" "./" "AUTHORS")
	# License
	BUNDLE_BASIC_INSTALL("../../" "./" "COPYING")

	Set(ExternalLibs libz.dylib libpng.dylib libfreetype.dylib libtiff.dylib
                libSDL.dylib libSDL_mixer.dylib libSDL_net.dylib
		libGLEW.dylib
		libjpeg.dylib
		libftgl.dylib
		libsmpeg.dylib
		)


	# We have to copy those libs
	FOREACH (LIB ${ExternalLibs})
		BUNDLE_ADD_LIB(${LIB})
	ENDFOREACH(LIB)

	# --- Resources ---
	# Sky
	Message(STATUS "[osx-bundle] Adding resources 'sky'...")
	BUNDLE_INSTALL_GLOB("sky" "sky" "*.tdf")
	# Shaders
	Message(STATUS "[osx-bundle] Adding resources 'shaders'...")
	BUNDLE_INSTALL_GLOB("shaders" "shaders" "*.vert")
	BUNDLE_INSTALL_GLOB("shaders" "shaders" "*.frag")
	# ai
	Message(STATUS "[osx-bundle] Adding resources 'ai'...")
	BUNDLE_INSTALL_GLOB("ai" "ai" "*.ai")
	# Gfx
	Message(STATUS "[osx-bundle] Adding resources 'gfx'...")
	BUNDLE_INSTALL_GLOB("gfx" "gfx" "*.jpg")
	BUNDLE_INSTALL_GLOB("gfx" "gfx" "*.png")
	BUNDLE_INSTALL_GLOB("gfx" "gfx" "*.tga")
	BUNDLE_INSTALL_GLOB("gfx/splash" "gfx/splash" "*.jpg")
	BUNDLE_INSTALL_GLOB("gfx/splash" "gfx/splash" "*.png")
	BUNDLE_INSTALL_GLOB("gfx/default_skin" "gfx/default_skin" "*.tga")
	BUNDLE_INSTALL_GLOB("gfx/default_skin" "gfx/default_skin" "*.jpg")
	BUNDLE_INSTALL_GLOB("gfx/default_skin" "gfx/default_skin" "*.png")
	BUNDLE_INSTALL_GLOB("gfx/mdrn_skin" "gfx/mdrn_skin" "*.tga")
	BUNDLE_INSTALL_GLOB("gfx/mdrn_skin" "gfx/mdrn_skin" "*.jpg")
	BUNDLE_INSTALL_GLOB("gfx/mdrn_skin" "gfx/mdrn_skin" "*.png")
	BUNDLE_INSTALL_GLOB("gfx/mdrn_teams" "gfx/mdrn_teams" "*.tga")
	BUNDLE_INSTALL_GLOB("gfx/mdrn_teams" "gfx/mdrn_teams" "*.jpg")
	BUNDLE_INSTALL_GLOB("gfx/mdrn_teams" "gfx/mdrn_teams" "*.png")
	BUNDLE_INSTALL_GLOB("gfx/sky" "gfx/sky" "*.jpg")
	BUNDLE_INSTALL_GLOB("gfx/teams" "gfx/teams" "*.tga")
	BUNDLE_INSTALL_GLOB("gfx/tactical_icons" "gfx/tactical_icons/" "*.tga")
	# gui
	Message(STATUS "[osx-bundle] Adding resources 'gui'...")
	BUNDLE_INSTALL_GLOB("gui" "gui" "*.tdf")
	BUNDLE_INSTALL_GLOB("gui" "gui" "*.area")
	BUNDLE_INSTALL_GLOB("gui" "gui" "*.skn")
	# Mods
	Message(STATUS "[osx-bundle] Adding resources 'mods'...")
	BUNDLE_INSTALL("mods/ta3d" "mods/ta3d" "files.txt")
	BUNDLE_INSTALL_GLOB("mods/ta3d/objects3d" "mods/ta3d/objects3d" "*.3dm")
	BUNDLE_INSTALL_GLOB("mods/ta3d/objects3d" "mods/ta3d/objects3d" "*.3do")
	# objects3d
	Message(STATUS "[osx-bundle] Adding resources 'objects3d'...")
	BUNDLE_INSTALL_GLOB("objects3d" "objects3d" "*.3dm")
	BUNDLE_INSTALL_GLOB("objects3d" "objects3d" "*.3do")
	# Fonts
	Message(STATUS "[osx-bundle] Adding resources 'fonts'...")
	BUNDLE_INSTALL_GLOB("fonts" "fonts" "*.ttf")
	# Scripts
	Message(STATUS "[osx-bundle] Adding resources 'scripts'...")
	BUNDLE_INSTALL_GLOB("scripts" "scripts" "*.lua")
	BUNDLE_INSTALL_GLOB("scripts/game" "scripts/game" "*.lua")
	BUNDLE_INSTALL_GLOB("scripts/game/ai" "scripts/ai" "*.lua")
	BUNDLE_INSTALL_GLOB("scripts" "scripts" "*.lh")
	# Pictures
	Message(STATUS "[osx-bundle] Adding resources 'pictures'...")
	BUNDLE_INSTALL_GLOB("pictures" "pictures" "*.jpg")
	BUNDLE_INSTALL_GLOB("pictures" "pictures" "*.ico")
	BUNDLE_INSTALL_GLOB("pictures" "pictures" "*.png")
	BUNDLE_INSTALL_GLOB("pictures" "pictures" "*.xcf")
	BUNDLE_INSTALL_GLOB("pictures" "pictures" "*.tga")
	BUNDLE_INSTALL_GLOB("pictures" "." "*.icns")
	# Intro
	Message(STATUS "[osx-bundle] Adding resources 'intro'...")
	BUNDLE_INSTALL_GLOB("resources/intro" "intro" "*.txt")
	# Translations
	Message(STATUS "[osx-bundle] Adding resources 'languages'...")
	BUNDLE_INSTALL_GLOB("resources/languages" "languages" "*.po")

	# OS X specific
	ADD_EXECUTABLE(${TA3D_BIN} MACOSX_BUNDLE ${TA3D_SOURCES})
	SET(GUI_TYPE MACOSX_BUNDLE)
	SET(MACOSX_BUNDLE_INFO_STRING            "")
	SET(MACOSX_BUNDLE_ICON_FILE              "ta3d.icns")
	SET(MACOSX_BUNDLE_GUI_IDENTIFIER         "org.ta3d.application")
	SET(MACOSX_BUNDLE_LONG_VERSION_STRING    "${${PACKAGE_VERSION}}")
	SET(MACOSX_BUNDLE_BUNDLE_NAME            "TA3D")
	SET(MACOSX_BUNDLE_SHORT_VERSION_STRING   "${PACKAGE_VERSION}")
	SET(MACOSX_BUNDLE_BUNDLE_VERSION         "${PACKAGE_VERSION}")
	SET(MACOSX_BUNDLE_COPYRIGHT              "Ta3D.org - GPLv2")

	# Relink
	FOREACH (LIB ${ExternalLibs})
		BUNDLE_RELINK("Contents/MacOS/ta3d" "${LIB}")
	ENDFOREACH(LIB)
	FOREACH (LIB ${ExternalLibs})
		FOREACH (SUBLIB ${ExternalLibs})
			BUNDLE_RELINK("Contents/Frameworks/${LIB}" "${SUBLIB}")
		ENDFOREACH(SUBLIB)
	ENDFOREACH(LIB)

ENDIF(NOT APPLE)


#
# --- Internal libraries ---
#
SET(LIBS  "${LIBS} ${FREETYPE_LIBRARIES}")
IF(WIN32)
	SET(${LIBS} "${LIBS} -llua -lgdi32 -lopengl32 -lglu32")
ENDIF(WIN32)

IF(APPLE)
	SET(${LIBS} "${LIBS} -Wl,'-headerpad FF'")
ENDIF(APPLE)

STRING(STRIP "${LIBS}" LIBS)
target_link_libraries(${TA3D_BIN} ${LIBS})


#
# Install
#
IF(NOT APPLE)
	install(TARGETS ${TA3D_BIN} RUNTIME DESTINATION ${TA3D_BINARIES_INSTALL_PATH})
ENDIF(NOT APPLE)

