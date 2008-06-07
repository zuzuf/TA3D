cmake_minimum_required(VERSION 2.4)

if( NOT CMAKE_MINOR_VERSION EQUAL 4 OR NOT CMAKE_MAJOR_VERSION EQUAL 2 )
	cmake_policy(SET CMP0004 OLD)
endif( NOT CMAKE_MINOR_VERSION EQUAL 4 OR NOT CMAKE_MAJOR_VERSION EQUAL 2 )

project(TA3D)

Include( FindOpenGL )

if( NOT OPENGL_FOUND )
	message( FATAL_ERROR "OpenGL libraries not found !!" )
endif( NOT OPENGL_FOUND )

Include( CheckIncludeFile )

CHECK_INCLUDE_FILE( allegro.h CMAKE_HAVE_ALLEGRO_H)
IF(NOT CMAKE_HAVE_ALLEGRO_H AND NOT CMAKE_CROSSCOMPILING )
	message( FATAL_ERROR "Allegro >= 4.2.2 is required" )
ENDIF(NOT CMAKE_HAVE_ALLEGRO_H AND NOT CMAKE_CROSSCOMPILING )

find_library(VAR_LIB_ALLEG_GL agl )
if( NOT VAR_LIB_ALLEG_GL )
	message( FATAL_ERROR "AllegroGL >= 0.4.1 is required" )
endif( NOT VAR_LIB_ALLEG_GL )

link_directories( ${TA3D_SOURCE_DIR}/src/lua/ ${TA3D_SOURCE_DIR}/src/hawknl/src/ )

set( LIBS "" )

string( LENGTH ${CMAKE_CXX_FLAGS} VA )

if( ${VA} EQUAL 1 )
	set( CMAKE_CXX_FLAGS "-O2 -g" )
endif( ${VA} EQUAL 1 )

message( STATUS "CXXFLAGS set to ${CMAKE_CXX_FLAGS}" )

if( WIN32 OR CMAKE_CROSSCOMPILING )
	set( ALLEGRO_CONFIG "-lalleg" )

	if( CMAKE_CROSSCOMPILING )
		SET( target i586-mingw32msvc- )
	endif( CMAKE_CROSSCOMPILING )

	if( NOT BUILT_LUA )
		execute_process( COMMAND make -C "src/lua/" clean )
		execute_process( COMMAND make -C "src/lua/" mingw CC=${CMAKE_C_COMPILER} RANLIB=${target}ranlib )
		set( BUILT_LUA 1 CACHE BOOL "LUA has been built" )
	endif( NOT BUILT_LUA )

	if( NOT BUILT_NL )
		execute_process( COMMAND make -C "src/hawknl/src/" -f makefile.mingw clean )
		execute_process( COMMAND make -C "src/hawknl/src/" -f makefile.mingw CC=${CMAKE_C_COMPILER} )
		set( BUILT_NL 1 CACHE BOOL "HawkNL has been built" )
	endif( NOT BUILT_NL )
	execute_process( COMMAND cp src/lua/lua51.dll ./ )
	execute_process( COMMAND cp src/hawknl/src/NL.dll ./ )
	set( LIBS lua agl alleg user32 gdi32 opengl32 glu32 z.dll NL ${OPENGL_LIBRARIES} )
elseif( UNIX )
	if( NOT BUILT_LUA )
		execute_process( COMMAND make -C "src/lua/" clean )
		execute_process( COMMAND make -C "src/lua/" linux )
		set( BUILT_LUA 1 CACHE BOOL "LUA has been built" )
	endif( NOT BUILT_LUA )

	if( NOT BUILT_NL )
		execute_process( COMMAND make -C "src/hawknl/src/" -f makefile.linux clean )
		execute_process( COMMAND make -C "src/hawknl/src/" -f makefile.linux )
		set( BUILT_NL 1 CACHE BOOL "HawkNL has been built" )
	endif( NOT BUILT_NL )

	execute_process( COMMAND allegro-config --libs release OUTPUT_VARIABLE ALLEGRO_CONFIG )
	set( LIBS agl lua z NL ${OPENGL_LIBRARIES} ${ALLEGRO_CONFIG} )

	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmmx -rdynamic -ffast-math" )
endif( WIN32 OR CMAKE_CROSSCOMPILING )

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${TA3D_SOURCE_DIR} )

# for compatibility with old versions of cmake
set( EXECUTABLE_OUTPUT_PATH ${TA3D_SOURCE_DIR} )

set( PACKAGE_BUGREPORT "zuzuf86@gmail.com" )
set( PACKAGE_VERSION "0.5.0 WIP" )
set( PACKAGE_STRING "TA3D ${PACKAGE_VERSION}" )
set( TA3D_ENGINE_VERSION "${PACKAGE_STRING}" )

configure_file( ${TA3D_SOURCE_DIR}/config.h.cmake ${TA3D_SOURCE_DIR}/config.h )

add_subdirectory(src)
