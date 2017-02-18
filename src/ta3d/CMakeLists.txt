PROJECT(TA3D)
cmake_minimum_required(VERSION 2.6)

message(STATUS "")
message(STATUS "--- TA3D ---")
message(STATUS "")


#
# --- Settings ---
#
# The maximum players
set(TA3D_MAX_PLAYERS_HARD_LIMIT  10)

set(CPACK_PACKAGE_VERSION_MAJOR "${TA3D_VERSION_HI}")
set(CPACK_PACKAGE_VERSION_MINOR "${TA3D_VERSION_LO}")
set(CPACK_PACKAGE_VERSION_PATCH "${TA3D_VERSION_PATCH}")

set(CPACK_PACKAGE_VERSION "${TA3D_VERSION_HI}.${TA3D_VERSION_LO}.${TA3D_VERSION_PATCH}.${TA3D_VERSION_TAG}")
set(CPACK_PACKAGE_NAME "TA3D")





#
# -----------------------------------------------------------------------------
#

if(SVN_REV)
	set(CPACK_PACKAGE_VERSION "${TA3D_VERSION_HI}.${TA3D_VERSION_LO}.${TA3D_VERSION_PATCH}.${TA3D_VERSION_TAG}.r${SVN_REV}")
endif(SVN_REV)



# Initialization
set(LIBS "")
set(PACKAGE_BUGREPORT "${TA3D_MAIL_BUGREPORT}")
set(PACKAGE_VERSION "${TA3D_VERSION_HI}.${TA3D_VERSION_LO}.${TA3D_VERSION_PATCH} ${TA3D_VERSION_TAG}")
set(PACKAGE_STRING "TA3D ${PACKAGE_VERSION}")
set(TA3D_ENGINE_VERSION "${PACKAGE_STRING}")


set(TA3D_COMMON_CPP_FLAGS  "")
if(NOT WIN32 OR MINGW)
	set(TA3D_COMMON_CPP_FLAGS  "${TA3D_COMMON_CPP_FLAGS} -W -Wextra -Wall -Wfloat-equal -Woverloaded-virtual")
	set(TA3D_COMMON_CPP_FLAGS  "${TA3D_COMMON_CPP_FLAGS} -Wconversion")
endif()
if(APPLE)
	set(TA3D_COMMON_CPP_FLAGS  "${TA3D_COMMON_CPP_FLAGS} -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64")
	set(TA3D_COMMON_CPP_FLAGS  "${TA3D_COMMON_CPP_FLAGS} -fvisibility=hidden")
endif()


#
# Command line options for G++ (Debug)
#
# Ex: cmake . -DCMAKE_BUILD_TYPE=release
#
if("${CMAKE_BUILD_TYPE}" STREQUAL "release")

	#
	# Build Configuration: Release
	#
	message(STATUS "Build Configuration: Release")

	if(NOT WIN32)
		Set(CMAKE_CXX_FLAGS "${TA3D_COMMON_CPP_FLAGS} -O2 -fomit-frame-pointer -mfpmath=sse -msse -msse2")
		message(STATUS "Default C++ flags set to `${CMAKE_CXX_FLAGS}`")
	Else(NOT WIN32)
		if(MINGW)
			set(CMAKE_CXX_FLAGS "${TA3D_COMMON_CPP_FLAGS} -O2 -fomit-frame-pointer")
		endif(MINGW)
		if(MSVC)
			set(CMAKE_CXX_FLAGS "${TA3D_COMMON_CPP_FLAGS} /Ob2 /Ot /O2 /MDd /Wp64 /Oy /MT /GL /GS-")
		endif(MSVC)
	endif(NOT WIN32)
	add_definitions("-DNDEBUG") # Remove asserts

Else("${CMAKE_BUILD_TYPE}" STREQUAL "release")

	#
	# Build Configuration: Debug
	#
	message(STATUS "Build Configuration: Debug")

	if(NOT WIN32)
		Set(CMAKE_CXX_FLAGS "${TA3D_COMMON_CPP_FLAGS} -g -ggdb2")
		If(APPLE)
			Set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -gfull")
		ENdif(APPLE)
	else(NOT WIN32)
		if(MINGW)
			Set(CMAKE_CXX_FLAGS "${TA3D_COMMON_CPP_FLAGS} -g2")
		endif(MINGW)
		if(MSVC)
			Set(CMAKE_CXX_FLAGS "/GR /Ob2 /Ot /MDd /MTd /fp:except")
		endif(MSVC)
	endif(NOT WIN32)
	add_definitions("-DLOGS_USE_DEBUG")

Endif("${CMAKE_BUILD_TYPE}" STREQUAL "release")





#
# Platform
#
if(NOT WIN32)
	if(APPLE)
		add_definitions("-DTA3D_PLATFORM_DARWIN -DTA3D_PLATFORM_MAC")
	else(APPLE)
		set(LINUX 1)
		add_definitions("-DTA3D_PLATFORM_LINUX")
	endif(APPLE)
	add_definitions("-DTA3D_PLATFORM_UNIX -DTA3D_PLATFORM_GCC")
else(NOT WIN32)
	add_definitions("-DTA3D_PLATFORM_WINDOWS -DTA3D_PLATFORM_WIN32")
endif(NOT WIN32)


Include(CheckIncludeFile)


#
# --- SDL ---
#
set(SDL_PREFIX "[SDL]")
Include(FindSDL)
Include(FindSDL_image)
Include(FindSDL_mixer)
Include(FindSDL_net)

# Manual setup for mingw32 (all required files are included in source tree)
#IF (WIN32 OR (CMAKE_CROSSCOMPILING AND NOT UNIX))

#	set( SDL_FOUND included )
#	set( SDLIMAGE_FOUND included )
#	set( SDLMIXER_FOUND included )
#	set( SDLNET_FOUND included )
#	set( SMPEG_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/src/tools/win32/mingw32/libs/libsmpeg.a )

#	include_DIRECTORIES( "${CMAKE_CURRENT_SOURCE_DIR}/src/tools/win32/mingw32/include" )

#	set( SDL_INCLUDE_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/src/tools/win32/mingw32/include/SDL")
#	set( SDLIMAGE_include_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/src/tools/win32/mingw32/include/SDL")
#	set( SDLMIXER_include_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/src/tools/win32/mingw32/include/SDL")
#	set( SDLNET_include_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/src/tools/win32/mingw32/include/SDL")

#	set( SDL_LIBRARY "mingw32;${CMAKE_CURRENT_SOURCE_DIR}/src/tools/win32/mingw32/libs/libSDLmain.a;${CMAKE_CURRENT_SOURCE_DIR}/src/tools/win32/mingw32/libs/SDL.dll")
#	set( SDLIMAGE_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/src/tools/win32/mingw32/libs/SDL_image.dll")
#	set( SDLMIXER_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/src/tools/win32/mingw32/libs/SDL_mixer.dll")
#	set( SDLNET_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/src/tools/win32/mingw32/libs/SDL_net.dll")

#endif (WIN32 OR (CMAKE_CROSSCOMPILING AND NOT UNIX))

	find_library(SMPEG_LIBRARY smpeg)
	message(STATUS "${SDL_PREFIX} SDL: ${SDL_FOUND}")
	message(STATUS "${SDL_PREFIX} SDL_image: ${SDLIMAGE_FOUND}")
	message(STATUS "${SDL_PREFIX} SDL_mixer: ${SDLMIXER_FOUND}")
	message(STATUS "${SDL_PREFIX} SDL_net: ${SDLNET_FOUND}")
	message(STATUS "${SDL_PREFIX} SMPEG: ${SMPEG_LIBRARY}")
	message(STATUS "${SDL_PREFIX} Include dir: ${SDL_INCLUDE_DIR}")
	message(STATUS "${SDL_PREFIX} The SDL library: ${SDL_LIBRARY}")
	message(STATUS "${SDL_PREFIX} The SDL_image library: ${SDLIMAGE_LIBRARY}")
	message(STATUS "${SDL_PREFIX} The SDL_mixer library: ${SDLMIXER_LIBRARY}")
	message(STATUS "${SDL_PREFIX} The SDL_net library: ${SDLNET_LIBRARY}")

if(NOT SDL_FOUND)
	message(FATAL_ERROR "SDL not found, SDL is required")
endif(NOT SDL_FOUND)
if(NOT SDLIMAGE_FOUND)
	message(FATAL_ERROR "SDL_image not found, SDL_image is required")
endif(NOT SDLIMAGE_FOUND)
if(NOT SDLMIXER_FOUND)
	message(FATAL_ERROR "SDL_mixer not found, SDL_mixer is required")
endif(NOT SDLMIXER_FOUND)
if(NOT SDLNET_FOUND)
	message(FATAL_ERROR "SDL_net not found, SDL_net is required")
endif(NOT SDLNET_FOUND)

LINK_LIBRARIES(${SDL_LIBRARY})
LINK_LIBRARIES(${SDLIMAGE_LIBRARY})
LINK_LIBRARIES(${SDLMIXER_LIBRARY})
LINK_LIBRARIES(${SDLNET_LIBRARY})
LINK_LIBRARIES(${SMPEG_LIBRARY})

include_DIRECTORIES(${SDL_INCLUDE_DIR})
include_DIRECTORIES(${SDLIMAGE_include_DIR})
include_DIRECTORIES(${SDLMIXER_include_DIR})
include_DIRECTORIES(${SDLNET_include_DIR})

link_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/lua/")


#
# --- LUA / LUAJIT ---
#
if(APPLE)
	# On OS X, we will directly use luajit from macports
	set(LUA_NOJIT 1)
	execute_process(COMMAND pkg-config lua --cflags OUTPUT_VARIABLE LUA_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND pkg-config lua --libs OUTPUT_VARIABLE LUA_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LUA_CFLAGS}")
	set(LIBS "${LUA_LIBS} -lz")
	message(STATUS "lua cflags : ${LUA_CFLAGS}")
	message(STATUS "lua libs   : ${LUA_LIBS}")
else()
	if(LUA_NOJIT)
		if(WIN32 OR (CMAKE_CROSSCOMPILING AND NOT UNIX))

			if( CMAKE_CROSSCOMPILING )
				set( target i586-mingw32msvc- )
				set( MAKE_WIN "make")
				set( RANLIB_WIN "${target}ranlib")
			else( CMAKE_CROSSCOMPILING )
				FIND_PROGRAM(MAKE_WIN "mingw32-make.exe")
				FIND_PROGRAM(RANLIB_WIN "ranlib.exe")
			endif( CMAKE_CROSSCOMPILING )

			if( NOT BUILT_LUA )
				message(STATUS "Building Lua")
				execute_process(WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/lua" COMMAND ${MAKE_WIN} clean)
				execute_process(WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/lua" COMMAND ${MAKE_WIN} mingw CC=${CMAKE_C_COMPILER} RANLIB=${RANLIB_WIN})
				set( BUILT_LUA 1 CACHE BOOL "Lua has been built" )
			endif( NOT BUILT_LUA )

			execute_process( COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/src/lua/lua51.dll" "${CMAKE_CURRENT_SOURCE_DIR}/" )

			if( NOT BUILT_LUA )
				execute_process( COMMAND make -C "${CMAKE_CURRENT_SOURCE_DIR}/src/lua/" clean)
				execute_process( COMMAND make -C "${CMAKE_CURRENT_SOURCE_DIR}/src/lua/" mingw)
				set( BUILT_LUA 1 CACHE BOOL "Lua has been built")
			endif( NOT BUILT_LUA )

			set( LIBS "-L./lua -llua -luser32 -lgdi32 -lopengl32 -lglu32 -lz")

			if(NOT CMAKE_CROSSCOMPILING)
				set( LIBS "${LIBS} -lSDLmain")
			endif(NOT CMAKE_CROSSCOMPILING)

#			LINK_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/src/tools/win32/mingw32/libs/")

		else(WIN32 OR (CMAKE_CROSSCOMPILING AND NOT UNIX))

			if(NOT APPLE)
				if( NOT BUILT_LUA )
					execute_process( COMMAND make -C "ta3d/src/lua/" clean )
					execute_process( COMMAND make -C "ta3d/src/lua/" linux CC=${CMAKE_C_COMPILER} )
					set( BUILT_LUA 1 CACHE BOOL "Lua has been built" )
				endif( NOT BUILT_LUA )
			endif(NOT APPLE)

			set(LIBS "-llua -lz -ldl")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -ffast-math")

		endif(WIN32 OR (CMAKE_CROSSCOMPILING AND NOT UNIX))
	else(LUA_NOJIT)			# LuaJIT 2
		if(WIN32 OR (CMAKE_CROSSCOMPILING AND NOT UNIX))
			if( CMAKE_CROSSCOMPILING )
				set( target i586-mingw32msvc- )
				set( MAKE_WIN "make")
				set( RANLIB_WIN "${target}ranlib")
			else( CMAKE_CROSSCOMPILING )
				FIND_PROGRAM(MAKE_WIN "mingw32-make.exe")
				FIND_PROGRAM(RANLIB_WIN "ranlib.exe")
			endif( CMAKE_CROSSCOMPILING )

			if( NOT BUILT_LUAJIT )
				message(STATUS "Building LuaJIT")
				execute_process(WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/luajit" COMMAND ${MAKE_WIN} clean)
				execute_process(WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/luajit" COMMAND ${MAKE_WIN} CC=${CMAKE_C_COMPILER} RANLIB=${RANLIB_WIN})
				set( BUILT_LUAJIT 1 CACHE BOOL "LuaJIT has been built" )
			endif( NOT BUILT_LUAJIT )

			set( LIBS "-L${CMAKE_CURRENT_SOURCE_DIR}/src/luajit/src -lluajit -luser32 -lgdi32 -lopengl32 -lglu32 -lz")

			if(NOT CMAKE_CROSSCOMPILING)
				set( LIBS "${LIBS} -lSDLmain")
			endif(NOT CMAKE_CROSSCOMPILING)

#		LINK_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/src/tools/win32/mingw32/libs/")

		else(WIN32 OR (CMAKE_CROSSCOMPILING AND NOT UNIX))

			if( NOT BUILT_LUAJIT )
				execute_process(COMMAND lua -v RESULT_VARIABLE LUA_PROGRAM_ERROR)
				if (LUA_PROGRAM_ERROR)
					message(FATAL_ERROR "No Lua interpreter found! A Lua interpreter is required to build LuaJIT")
				endif (LUA_PROGRAM_ERROR)

				execute_process( COMMAND make -C "${CMAKE_CURRENT_SOURCE_DIR}/src/luajit/" clean )
				if(CMAKE_CROSSCOMPILING)
					set(HOST_CC gcc)
				else(CMAKE_CROSSCOMPILING)
					set(HOST_CC ${CMAKE_C_COMPILER})
				endif(CMAKE_CROSSCOMPILING)
				execute_process( COMMAND make -C "${CMAKE_CURRENT_SOURCE_DIR}/src/luajit/" CC=${CMAKE_C_COMPILER} HOST_CC=${HOST_CC} )
				set( BUILT_LUAJIT 1 CACHE BOOL "LuaJIT has been built" )
			endif( NOT BUILT_LUAJIT )

			set(LIBS "-L${CMAKE_CURRENT_SOURCE_DIR}/src/luajit/src -lluajit -lz -ldl")
			if(NOT APPLE)
				set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -ffast-math")
			endif(NOT APPLE)

		endif(WIN32 OR (CMAKE_CROSSCOMPILING AND NOT UNIX))
	endif()
endif()

if(LUA_NOJIT)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLUA_NOJIT")
endif(LUA_NOJIT)



#
# --- GLEW ---
#
if(NOT APPLE)
        if(WIN32 OR (CMAKE_CROSSCOMPILING AND NOT UNIX))
#                set(GLEW_LIB "${CMAKE_CURRENT_SOURCE_DIR}/src/tools/win32/mingw32/libs/libglew32.dll.a")
                find_library(GLEW_LIB GLEW32)
                if(NOT GLEW_LIB)
                        find_library(GLEW_LIB glew32)
                endif(NOT GLEW_LIB)
                find_file(GLEW_HEADER GL/glew.h)
                if(NOT GLEW_HEADER)
                        message(FATAL_ERROR "glew.h not found!")
                endif(NOT GLEW_HEADER)
                message(STATUS "GLEW Header : ${GLEW_HEADER}")
        else(WIN32 OR (CMAKE_CROSSCOMPILING AND NOT UNIX))
		find_library(GLEW_LIB GLEW)
		if(NOT GLEW_LIB)
			find_library(GLEW_LIB glew)
		endif(NOT GLEW_LIB)
		find_file(GLEW_HEADER GL/glew.h)
		if(NOT GLEW_HEADER)
			message(FATAL_ERROR "glew.h not found!")
		endif(NOT GLEW_HEADER)
		message(STATUS "GLEW Header : ${GLEW_HEADER}")
        endif(WIN32 OR (CMAKE_CROSSCOMPILING AND NOT UNIX))
        message(STATUS "GLEW Library : ${GLEW_LIB}")
else()
	execute_process(COMMAND pkg-config glew --libs    OUTPUT_VARIABLE GLEW_LIB    OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND pkg-config glew --cflags  OUTPUT_VARIABLE GLEW_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
	message(STATUS "GLEW Library : ${GLEW_LIB}")
	message(STATUS "GLEW CFLags : ${GLEW_CFLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GLEW_CFLAGS}")
endif()
if(NOT GLEW_LIB)
	message(FATAL_ERROR "GLEW library not found!")
endif(NOT GLEW_LIB)
set(LIBS "${LIBS} ${GLEW_LIB}")

#
# --- GLU ---
#
if(APPLE)
	include_directories("/System/Library/Frameworks/OpenGL.framework/Headers")
	set(LIBS "${LIBS} -L/System/Library/Frameworks/OpenGL.framework/Libraries -lGL -lGLU")
else()
	if(UNIX)
		find_library(GLU_LIB GLU)
		if(NOT GLU_LIB)
			find_library(GLEW_LIB glu)
		endif(NOT GLU_LIB)
		message(STATUS "GLU Library : ${GLU_LIB}")
        set(LIBS "${LIBS} ${GLU_LIB} -lGL")
	endif(UNIX)
endif()


#
# --- FTGL ---
#
if(NOT APPLE)
    if(WIN32 OR (CMAKE_CROSSCOMPILING AND NOT UNIX))
#        set( FREETYPE_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/tools/win32/mingw32/include/freetype2/")
#        set( FREETYPE_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/src/tools/win32/mingw32/libs/libfreetype.dll.a")
#        message(STATUS "FreeType Library : ${FREETYPE_LIBRARIES}")
        FIND_PACKAGE(Freetype)
    else(WIN32 OR (CMAKE_CROSSCOMPILING AND NOT UNIX))
        FIND_PACKAGE(Freetype)
    endif(WIN32 OR (CMAKE_CROSSCOMPILING AND NOT UNIX))

    include_directories( ${FREETYPE_INCLUDE_DIRS} )

    set(FTGL_LIB ${CMAKE_CURRENT_SOURCE_DIR}/../libs/ftgl/lib/libftgl.a)
    include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../libs/ftgl/include )
    set(__FTGL__lower__ true)

    message(STATUS "FTGL Library : ${FTGL_LIB}")
    set(LIBS "${LIBS} ${FTGL_LIB}")

else(NOT APPLE)

	execute_process(COMMAND pkg-config --libs  ftgl OUTPUT_VARIABLE FTGL_LIB OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND pkg-config --cflags  ftgl OUTPUT_VARIABLE FTGL_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
	message(STATUS "FTGL Library : ${FTGL_LIB}")
	message(STATUS "FTGL CFLags : ${FTGL_CFLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FTGL_CFLAGS}")
	set(LIBS "${LIBS} ${FTGL_LIB}")
	set(__FTGL__lower__ false)

endif(NOT APPLE)


#
# --- SDL_sgui ---
#
set(SDL_SGUI_LIB ${CMAKE_CURRENT_SOURCE_DIR}/../libs/SDL_sgui/libSDL_sgui.a)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../libs/SDL_sgui/include )

message(STATUS "SDL_sgui Library : ${SDL_SGUI_LIB}")
set(LIBS "${LIBS} ${SDL_SGUI_LIB}")


#
# --- ZLib ---
#
#if(WIN32)
#	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${CMAKE_CURRENT_SOURCE_DIR}/src/tools/win32/mingw32/include/zlib")
#endif(WIN32)

#
# --- OpenMP ---
#
Include(FindOpenMP)
if(DISABLE_OPENMP)
	set(OPENMP_FOUND 0)
endif(DISABLE_OPENMP)

if(OPENMP_FOUND)
	message(STATUS "OpenMP found")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
else(OPENMP_FOUND)
	message(STATUS "OpenMP not found")
endif(OPENMP_FOUND)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
# for compatibility with old versions of cmake
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})


#
# --- CPack: Installation process ---
#
# OS X Note : There is no need of CPake under OS X. On this platform,
# an application is an independant bundle that embeds all needed data files.
# A graphical application is only working if launched from its bundle.
# So We cannot wait to install the application to be able to run it.
#
if(NOT APPLE)

	if(NOT WIN32 AND UNIX)
		message("-- TA3D will be installed in: ${CMAKE_INSTALL_PREFIX}")
	endif(NOT WIN32 AND UNIX)

	set(TA3D_RESOURCES_INSTALL_PATH "share/games/ta3d")
	set(TA3D_BINARIES_INSTALL_PATH "games")

	install(DIRECTORY ../../docs DESTINATION ${TA3D_RESOURCES_INSTALL_PATH} PATTERN ".*" EXCLUDE PATTERN "*~" EXCLUDE)
	install(DIRECTORY fonts DESTINATION ${TA3D_RESOURCES_INSTALL_PATH} PATTERN ".*" EXCLUDE PATTERN "*~" EXCLUDE)
	install(DIRECTORY gfx DESTINATION ${TA3D_RESOURCES_INSTALL_PATH} PATTERN ".*" EXCLUDE PATTERN "*~" EXCLUDE)
	install(DIRECTORY gui DESTINATION ${TA3D_RESOURCES_INSTALL_PATH} PATTERN ".*" EXCLUDE PATTERN "*~" EXCLUDE)
	install(DIRECTORY objects3d DESTINATION ${TA3D_RESOURCES_INSTALL_PATH} PATTERN ".*" EXCLUDE PATTERN "*~" EXCLUDE)
	install(DIRECTORY resources DESTINATION ${TA3D_RESOURCES_INSTALL_PATH} PATTERN ".*" EXCLUDE PATTERN "*~" EXCLUDE)
	install(DIRECTORY scripts DESTINATION ${TA3D_RESOURCES_INSTALL_PATH} PATTERN ".*" EXCLUDE PATTERN "*~" EXCLUDE PATTERN "__campaign_script.lua" EXCLUDE)
	install(DIRECTORY shaders DESTINATION ${TA3D_RESOURCES_INSTALL_PATH} PATTERN ".*" EXCLUDE PATTERN "*~" EXCLUDE)
	install(DIRECTORY sky DESTINATION ${TA3D_RESOURCES_INSTALL_PATH} PATTERN ".*" EXCLUDE PATTERN "*~" EXCLUDE)

	# copy the file that will add TA3D and 3DMEditor to the desktop menu
	install(FILES distrib/linux/ta3d DESTINATION share/menu)
	install(FILES distrib/linux/ta3d.desktop DESTINATION share/applications)
	install(FILES distrib/linux/ta3d_cfg.desktop DESTINATION share/applications)
	install(FILES ota_resources_installer.sh DESTINATION ${TA3D_BINARIES_INSTALL_PATH})
	install(FILES distrib/linux/ta3d_installer.desktop DESTINATION share/applications)
	install(FILES ta3d.png DESTINATION ${TA3D_RESOURCES_INSTALL_PATH})
	install(FILES ta3d_cfg.png DESTINATION ${TA3D_RESOURCES_INSTALL_PATH})
	install(FILES readme.html DESTINATION ${TA3D_RESOURCES_INSTALL_PATH})
	install(FILES readme_fr.html DESTINATION ${TA3D_RESOURCES_INSTALL_PATH})
	install(FILES ../../TODO DESTINATION ${TA3D_RESOURCES_INSTALL_PATH})
	install(FILES ../../AUTHORS DESTINATION ${TA3D_RESOURCES_INSTALL_PATH})
	install(FILES ../../README DESTINATION ${TA3D_RESOURCES_INSTALL_PATH})
	install(FILES ../../NEWS DESTINATION ${TA3D_RESOURCES_INSTALL_PATH})
	install(FILES ta3d.mod DESTINATION ${TA3D_RESOURCES_INSTALL_PATH})
	install(FILES ../../ChangeLog DESTINATION ${TA3D_RESOURCES_INSTALL_PATH})

    #
	# --- CPACK configuration ---
	#
	STRING(TOLOWER ${CMAKE_BUILD_TYPE} BTYPE)

	set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "TA3D (Total Annihilation 3D), RTS Engine")
	set(CPACK_PACKAGE_VENDOR "the TA3D team")
	set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../../README")
	set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../../COPYING")
	set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PACKAGE_STRING}")
	set(CPACK_GENERATOR "STGZ;DEB;RPM")
	set(CPACK_PACKAGE_CONTACT "${TA3D_MAIL_BUGREPORT}")
	set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Roland Brochard")
	set(CPACK_DEBIAN_PACKAGE_SECTION "games")
	if(NOT ${BTYPE} STREQUAL "debug")
		set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsdl1.2debian, libsdl-image1.2, libsdl-mixer1.2, libsdl-net1.2, libz1, libglew1.5, libfreetype6, p7zip-full, p7zip-rar")
	else(NOT ${BTYPE} STREQUAL "debug")
		set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsdl1.2debian, libsdl-image1.2, libsdl-mixer1.2, libsdl-net1.2, libz1, libglew1.5, libfreetype6, p7zip-full, p7zip-rar, gdb")
	endif(NOT ${BTYPE} STREQUAL "debug")

	if(NOT WIN32 AND UNIX)
		if(NOT ${BTYPE} STREQUAL "debug")
			set(CPACK_STRIP_FILES "ta3d")
		endif(NOT ${BTYPE} STREQUAL "debug")
		set(CPACK_SOURCE_STRIP_FILES "")
	endif(NOT WIN32 AND UNIX)
	set(CPACK_PACKAGE_EXECUTABLES "ta3d")
	include(CPack)

else(NOT APPLE)

	# Specific Settings for the OS X Bundle
	set(GUI_TYPE MACOSX_BUNDLE)
	set(MACOSX_BUNDLE_BUNDLE_NAME "TA3D")
	set(MACOSX_BUNDLE_SHORT_VERSION_STRING "ta3d")

endif(NOT APPLE)



#
# --- config.h ---
#
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_SOURCE_DIR}/config.h )
if(APPLE)
	# Version given by Informations that CMake already has
	configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/distrib/macos/version.cmake ${CMAKE_CURRENT_SOURCE_DIR}/distrib/macos/config.sh )
endif(APPLE)




# The source code
ADD_SUBDIRECTORY(src)

