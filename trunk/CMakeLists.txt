cmake_minimum_required(VERSION 2.4)

if( NOT CMAKE_MINOR_VERSION EQUAL 4 OR NOT CMAKE_MAJOR_VERSION EQUAL 2 )
	cmake_policy(SET CMP0004 OLD)
endif( NOT CMAKE_MINOR_VERSION EQUAL 4 OR NOT CMAKE_MAJOR_VERSION EQUAL 2 )

PROJECT(TA3D)



Include( CheckIncludeFile )


#
# OpenGL
#
Include( FindOpenGL )
if( NOT OPENGL_FOUND )
	message( FATAL_ERROR "OpenGL libraries not found !!" )
endif( NOT OPENGL_FOUND )

#
# Allegro
#
IF(APPLE) # FIXME : Should be UNIX here
    FIND_PROGRAM(ALLEGRO_CONFIG_BIN NAME allegro-config PATHS /opt/local/bin /opt/sw/bin)
    IF(${ALLEGRO_CONFIG_BIN} MATCHES "ALLEGRO_CONFIG_BIN-NOTFOUND")
        MESSAGE(FATAL_ERROR "Allegro >= 4.2.2 is required")
    ENDIF(${ALLEGRO_CONFIG_BIN} MATCHES "ALLEGRO_CONFIG_BIN-NOTFOUND")
    SET(CMAKE_HAVE_ALLEGRO_H 1)
	EXECUTE_PROCESS(COMMAND ${ALLEGRO_CONFIG_BIN} --cppflags release OUTPUT_VARIABLE ALLEGRO_CONFIG_FLAGS)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ALLEGRO_CONFIG_FLAGS}")
ELSE(APPLE)
    CHECK_INCLUDE_FILE( allegro.h CMAKE_HAVE_ALLEGRO_H)
    IF(NOT CMAKE_HAVE_ALLEGRO_H AND NOT CMAKE_CROSSCOMPILING )
	    MESSAGE( FATAL_ERROR "Allegro >= 4.2.2 is required" )
    ENDIF(NOT CMAKE_HAVE_ALLEGRO_H AND NOT CMAKE_CROSSCOMPILING )
ENDIF(APPLE)


find_library(VAR_LIB_ALLEG_GL agl )
if( NOT VAR_LIB_ALLEG_GL )
	message( FATAL_ERROR "AllegroGL >= 0.4.1 is required" )
endif( NOT VAR_LIB_ALLEG_GL )

link_directories( ${TA3D_SOURCE_DIR}/src/lua/ ${TA3D_SOURCE_DIR}/src/hawknl/src/ )

SET( LIBS "" )

#string( LENGTH ${CMAKE_CXX_FLAGS} VA )
#if( ${VA} EQUAL 1 )
#	SET( CMAKE_CXX_FLAGS "-O2 -g" )
#endif( ${VA} EQUAL 1 )
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -g" )


# message( STATUS "CXXFLAGS SET to ${CMAKE_CXX_FLAGS}" )

if( WIN32 OR CMAKE_CROSSCOMPILING )
	SET( ALLEGRO_CONFIG "-lalleg" )

	if( CMAKE_CROSSCOMPILING )
		SET( target i586-mingw32msvc- )
	endif( CMAKE_CROSSCOMPILING )

	if( NOT BUILT_LUA )
		EXECUTE_PROCESS( COMMAND make -C "src/lua/" clean )
		EXECUTE_PROCESS( COMMAND make -C "src/lua/" mingw CC=${CMAKE_C_COMPILER} RANLIB=${target}ranlib )
		SET( BUILT_LUA 1 CACHE BOOL "LUA has been built" )
	endif( NOT BUILT_LUA )

	if( NOT BUILT_NL )
		EXECUTE_PROCESS( COMMAND make -C "src/hawknl/src/" -f makefile.mingw clean )
		EXECUTE_PROCESS( COMMAND make -C "src/hawknl/src/" -f makefile.mingw CC=${CMAKE_C_COMPILER} )
		SET( BUILT_NL 1 CACHE BOOL "HawkNL has been built" )
	endif( NOT BUILT_NL )
	EXECUTE_PROCESS( COMMAND cp src/lua/lua51.dll ./ )
	EXECUTE_PROCESS( COMMAND cp src/hawknl/src/NL.dll ./ )
	SET( LIBS lua agl alleg user32 gdi32 opengl32 glu32 z.dll NL ${OPENGL_LIBRARIES} )
ELSE(UNIX)
    IF(NOT APPLE)
    	if( NOT BUILT_LUA )
	    	EXECUTE_PROCESS( COMMAND make -C "src/lua/" clean )
	    	EXECUTE_PROCESS( COMMAND make -C "src/lua/" linux )
	    	SET( BUILT_LUA 1 CACHE BOOL "LUA has been built" )
	    endif( NOT BUILT_LUA )
    ENDIF(NOT APPLE)

	if( NOT BUILT_NL )
        IF(LINUX)
		    EXECUTE_PROCESS( COMMAND make -C "src/hawknl/src/" -f makefile.linux clean )
		    EXECUTE_PROCESS( COMMAND make -C "src/hawknl/src/" -f makefile.linux )
        ENDIF(LINUX)
        IF(APPLE)
		    EXECUTE_PROCESS( COMMAND make -C "src/hawknl/src/" -f makefile.macosx clean )
		    EXECUTE_PROCESS( COMMAND make -C "src/hawknl/src/" -f makefile.macosx )
        ENDIF(APPLE)
		SET( BUILT_NL 1 CACHE BOOL "HawkNL has been built" )
	endif( NOT BUILT_NL )

	EXECUTE_PROCESS( COMMAND allegro-config --libs release OUTPUT_VARIABLE ALLEGRO_CONFIG )
	SET( LIBS agl lua z NL ${OPENGL_LIBRARIES} ${ALLEGRO_CONFIG} )

    IF(NOT APPLE)
	    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmmx -rdynamic -ffast-math")
    ENDIF(NOT APPLE)
ENDIF( WIN32 OR CMAKE_CROSSCOMPILING )

SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${TA3D_SOURCE_DIR} )

# for compatibility with old versions of cmake
SET( EXECUTABLE_OUTPUT_PATH ${TA3D_SOURCE_DIR} )

SET( PACKAGE_BUGREPORT "zuzuf86@gmail.com" )
SET( PACKAGE_VERSION "0.5.0 WIP" )
SET( PACKAGE_STRING "TA3D ${PACKAGE_VERSION}" )
SET( TA3D_ENGINE_VERSION "${PACKAGE_STRING}" )

CONFIGURE_FILE( ${TA3D_SOURCE_DIR}/config.h.cmake ${TA3D_SOURCE_DIR}/config.h )

ADD_SUBDIRECTORY(src)

