ADD_SUBDIRECTORY(logs)
ADD_SUBDIRECTORY(languages)
ADD_SUBDIRECTORY(threads)
ADD_SUBDIRECTORY(misc)
ADD_SUBDIRECTORY(gfx)
ADD_SUBDIRECTORY(input)
ADD_SUBDIRECTORY(network)
ADD_SUBDIRECTORY(ai)
ADD_SUBDIRECTORY(ingame)
ADD_SUBDIRECTORY(scripts)
ADD_SUBDIRECTORY(sounds)
ADD_SUBDIRECTORY(converters)
ADD_SUBDIRECTORY(animator)


LINK_LIBRARIES("${LIBS}")



# --------------------------------------------------------------------
# --- STDAFX
# --------------------------------------------------------------------
SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(SOURCES stdafx.h stdafx.cpp)
ADD_LIBRARY(stdafx STATIC ${SOURCES})

# --------------------------------------------------------------------
# --- Debug Informations
# --------------------------------------------------------------------
SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(SOURCES backtrace.cpp backtrace.h
            cError.cpp cError.h)
ADD_LIBRARY(debuginfos STATIC ${SOURCES})

# --------------------------------------------------------------------
# --- Console
# --------------------------------------------------------------------
SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(SOURCES console.cpp console.h)
ADD_LIBRARY(console STATIC ${SOURCES})

# --------------------------------------------------------------------
# --- GUI
# --------------------------------------------------------------------
SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(SOURCES gui.cpp gui.h)
ADD_LIBRARY(oldgui STATIC ${SOURCES})

# --------------------------------------------------------------------
# --- File formats
# --------------------------------------------------------------------
SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(SOURCES gaf.cpp gaf.h
            3do.cpp 3do.h
            3ds.cpp 3ds.h)
ADD_LIBRARY(fileformats STATIC ${SOURCES})

# --------------------------------------------------------------------
# --- HPI
# --------------------------------------------------------------------
SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(SOURCES TA3D_hpi.cpp TA3D_hpi.h
            hpiview.cpp)
ADD_LIBRARY(hpi STATIC ${SOURCES})






# --------------------------------------------------------------------
# --- EDITOR
# --------------------------------------------------------------------


if( WIN32 OR CMAKE_CROSSCOMPILING )
	SET( 3DMEDITOR_BIN 3dmeditor )
else( WIN32 OR CMAKE_CROSSCOMPILING )
	SET( 3DMEDITOR_BIN 3dmeditor )
endif( WIN32 OR CMAKE_CROSSCOMPILING )

ADD_EXECUTABLE(${3DMEDITOR_BIN}
		3dmeditor.cpp ta3d.h 3dmeditor_sub.cpp 3dmeditor.h
		stdafx.h TA3D_NameSpace.cpp TA3D_NameSpace.h ta3dbase.h)
target_link_libraries(${3DMEDITOR_BIN}
		stdafx logs gfx input console debuginfos 3dtoolbox misc i18n
		fileformats particles hpi oldgui gui ingame tdf
		converters animator gfx ta3dmath audio threads)



SET(SRC_ENGINE
		UnitEngine.cpp UnitEngine.h
		EngineClass.cpp EngineClass.h
		engine/weapondata.h  engine/weapondata.hxx)


# --------------------------------------------------------------------
# --- TA3D
# --------------------------------------------------------------------
IF(APPLE)
	SET(TA3D_BIN ta3d)
ELSE(APPLE)
	SET(TA3D_BIN ta3d)
ENDIF(APPLE)
SET(SOURCES
		${SRC_ENGINE}
		ta3d.h
		install.cpp
		engine.cpp engine.h
		restore.cpp restore.h
		menu.cpp menu.h
		fbi.cpp fbi.h
		intro.cpp intro.h
		tdf.cpp tdf.h tnt.cpp tnt.h
		stdafx.h TA3D_NameSpace.cpp TA3D_NameSpace.h ta3dbase.h
		main.cpp
		)



IF(NOT APPLE)
	ADD_EXECUTABLE(${TA3D_BIN} ${SOURCES})
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
		SET(SOURCES ${SOURCES} "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Resources/${relfolder}/${filename}")
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
			SET(SOURCES ${SOURCES} "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Resources/${relfolder}/${fl}")
		EndForEach(fl ${lst})
	ENDMACRO(BUNDLE_INSTALL_GLOB)

	MACRO(BUNDLE_BASIC_INSTALL srcfolder relfolder filename)
		File(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/${relfolder}/")
		ADD_CUSTOM_COMMAND(OUTPUT "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/${relfolder}/${filename}"
			COMMAND cp ARGS -f
				"${PROJECT_SOURCE_DIR}/${srcfolder}/${filename}"
				"${PROJECT_SOURCE_DIR}/ta3d.app/Contents/${relfolder}/${filename}"
			DEPENDS "${PROJECT_SOURCE_DIR}/${srcfolder}/${filename}")
		SET(SOURCES ${SOURCES} "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/${relfolder}/${filename}")
	ENDMACRO(BUNDLE_BASIC_INSTALL)

	MACRO(BUNDLE_ADD_LIB libname)
		ADD_CUSTOM_COMMAND(OUTPUT "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Frameworks/${libname}"
			COMMAND cp ARGS -fH "/opt/local/lib/${libname}" "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Frameworks/${libname}"
			DEPENDS "/opt/local/lib/${libname}")
		SET(SOURCES ${SOURCES} "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Frameworks/${libname}")
	ENDMACRO(BUNDLE_ADD_LIB)

	MACRO(BUNDLE_RELINK libname)
		ADD_CUSTOM_COMMAND(TARGET ${TA3D_BIN} POST_BUILD
			COMMAND install_name_tool ARGS -change '/opt/local/lib/${libname}' @executable_path/../Frameworks/${libname} ${PROJECT_SOURCE_DIR}/ta3d.app/Contents/MacOS/ta3d )
	ENDMACRO(BUNDLE_RELINK)

	File(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Resources/")
	File(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Frameworks/")
	File(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/MacOS/")

	# Ta3d mod
	BUNDLE_BASIC_INSTALL("./" "MacOS" "ta3d.mod")
	# Misc
	BUNDLE_BASIC_INSTALL("./" "./" "README")
	BUNDLE_BASIC_INSTALL("./" "./" "AUTHORS")
	# License
	BUNDLE_BASIC_INSTALL("./" "./" "COPYING")

	# LibZ
	BUNDLE_ADD_LIB("libz.1.dylib")
	BUNDLE_ADD_LIB("libSDL-1.2.0.dylib")
	BUNDLE_ADD_LIB("libSDL_mixer-1.2.0.dylib")
	BUNDLE_ADD_LIB("libSDL_image-1.2.0.dylib")
	BUNDLE_ADD_LIB("libSDL_net-1.2.0.dylib")
	BUNDLE_ADD_LIB("libGLEW.1.5.1.dylib")
	BUNDLE_ADD_LIB("libpng12.0.dylib")

	# --- Resources ---
	# Sky
	Message(STATUS "[bundle] Adding resources 'sky'...")
	BUNDLE_INSTALL_GLOB("sky" "sky" "*.tdf")
	# Shaders
	Message(STATUS "[bundle] Adding resources 'shaders'...")
	BUNDLE_INSTALL_GLOB("shaders" "shaders" "*.vert")
	BUNDLE_INSTALL_GLOB("shaders" "shaders" "*.frag")
	# ai
	Message(STATUS "[bundle] Adding resources 'ai'...")
	BUNDLE_INSTALL_GLOB("ai" "ai" "*.ai")
	# Gfx
	Message(STATUS "[bundle] Adding resources 'gfx'...")
	BUNDLE_INSTALL_GLOB("gfx" "gfx" "*.jpg")
	BUNDLE_INSTALL_GLOB("gfx" "gfx" "*.png")
	BUNDLE_INSTALL_GLOB("gfx" "gfx" "*.tga")
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
	Message(STATUS "[bundle] Adding resources 'gui'...")
	BUNDLE_INSTALL_GLOB("gui" "gui" "*.tdf")
	BUNDLE_INSTALL_GLOB("gui" "gui" "*.area")
	BUNDLE_INSTALL_GLOB("gui" "gui" "*.skn")
	# Mods
	Message(STATUS "[bundle] Adding resources 'mods'...")
	BUNDLE_INSTALL("mods/ta3d" "mods/ta3d" "files.txt")
	BUNDLE_INSTALL_GLOB("mods/ta3d/objects3d" "mods/ta3d/objects3d" "*.3dm")
	BUNDLE_INSTALL_GLOB("mods/ta3d/objects3d" "mods/ta3d/objects3d" "*.3do")
	# objects3d
	Message(STATUS "[bundle] Adding resources 'objects3d'...")
	BUNDLE_INSTALL_GLOB("objects3d" "objects3d" "*.3dm")
	BUNDLE_INSTALL_GLOB("objects3d" "objects3d" "*.3do")
	# Fonts
	Message(STATUS "[bundle] Adding resources 'fonts'...")
	BUNDLE_INSTALL_GLOB("fonts" "fonts" "*.ttf")
	# Scripts
	Message(STATUS "[bundle] Adding resources 'scripts'...")
	BUNDLE_INSTALL_GLOB("scripts" "scripts" "*.lua")
	BUNDLE_INSTALL_GLOB("scripts/game" "scripts/game" "*.lua")
	BUNDLE_INSTALL_GLOB("scripts/game/ai" "scripts/ai" "*.lua")
	BUNDLE_INSTALL_GLOB("scripts" "scripts" "*.lh")
	# Pictures
	Message(STATUS "[bundle] Adding resources 'pictures'...")
	BUNDLE_INSTALL_GLOB("pictures" "pictures" "*.jpg")
	BUNDLE_INSTALL_GLOB("pictures" "pictures" "*.ico")
	BUNDLE_INSTALL_GLOB("pictures" "pictures" "*.png")
	BUNDLE_INSTALL_GLOB("pictures" "pictures" "*.xcf")
	BUNDLE_INSTALL_GLOB("pictures" "pictures" "*.tga")
	BUNDLE_INSTALL_GLOB("pictures" "pictures" "*.icns")
	# Intro
	Message(STATUS "[bundle] Adding resources 'intro'...")
	BUNDLE_INSTALL_GLOB("resources/intro" "intro" "*.txt")
	# Translations
	Message(STATUS "[bundle] Adding resources 'languages'...")
	BUNDLE_INSTALL_GLOB("resources/languages" "languages" "*.txt")

	# *.res
	BUNDLE_BASIC_INSTALL("resources" "./resources" "ta3d.res")
	BUNDLE_BASIC_INSTALL("resources" "./resources" "3dmeditor.res")

	# OS X specific
	ADD_EXECUTABLE(${TA3D_BIN} MACOSX_BUNDLE ${SOURCES})
	SET(GUI_TYPE MACOSX_BUNDLE)
	SET(MACOSX_BUNDLE_BUNDLE_NAME "TA3D")
	SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "ta3d")
	SET(MACOSX_BUNDLE_ICON_FILE "pictures/ta3d.icns")

	# Relink
	BUNDLE_RELINK("libz.1.dylib")
	BUNDLE_RELINK("libSDL-1.2.0.dylib")
	BUNDLE_RELINK("libSDL_image-1.2.0.dylib")
	BUNDLE_RELINK("libSDL_mixer-1.2.0.dylib")
	BUNDLE_RELINK("libSDL_net-1.2.0.dylib")
	BUNDLE_RELINK("libGLEW.1.5.1.dylib")
	BUNDLE_RELINK("libpng12.0.dylib")

ENDIF(NOT APPLE)


#
# --- Internal libraries ---
#
target_link_libraries(${TA3D_BIN}
	stdafx input logs console i18n ingame 3dtoolbox menus gfx
	fileformats particles weapons ai audio hpi oldgui gui network misc scripts ta3dmath tdf
	converters threads debuginfos)




#
# --- External libraries
#

target_link_libraries(${TA3D_BIN} ${FTGL_LIB} ${FREETYPE_LIBRARIES})
target_link_libraries(${3DMEDITOR_BIN} ${FTGL_LIB} ${FREETYPE_LIBRARIES})

IF(WIN32)
	target_link_libraries(${3DMEDITOR_BIN} lua gdi32 opengl32 glu32 ${LIBS})
	target_link_libraries(${TA3D_BIN} lua gdi32 opengl32 glu32 ${LIBS})
ENDIF(WIN32)

IF(NOT APPLE)
	install(TARGETS ${TA3D_BIN} RUNTIME DESTINATION ${TA3D_BINARIES_INSTALL_PATH})
	install(TARGETS ${3DMEDITOR_BIN} RUNTIME DESTINATION ${TA3D_BINARIES_INSTALL_PATH})
ENDIF(NOT APPLE)

