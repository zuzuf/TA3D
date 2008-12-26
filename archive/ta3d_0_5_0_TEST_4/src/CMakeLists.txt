ADD_SUBDIRECTORY(logs)
ADD_SUBDIRECTORY(languages)
ADD_SUBDIRECTORY(threads)
ADD_SUBDIRECTORY(misc)
ADD_SUBDIRECTORY(jpeg)
ADD_SUBDIRECTORY(gfx)
ADD_SUBDIRECTORY(network)
ADD_SUBDIRECTORY(ai)
ADD_SUBDIRECTORY(ingame)
ADD_SUBDIRECTORY(scripts)


IF(APPLE)
    LINK_DIRECTORIES("tools/darwin/fmod/4.14.07/lib/")
    SET(GUI_TYPE MACOSX_BUNDLE)
    SET(MACOSX_BUNDLE_BUNDLE_NAME "TA3D")
    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "ta3d")
    LINK_LIBRARIES("-framework AGL -framework OpenGL -framework Carbon")
ENDIF(APPLE)
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
            cError.cpp cError.h
            TA3D_Exception.cpp TA3D_Exception.h)
ADD_LIBRARY(debuginfos STATIC ${SOURCES})

# --------------------------------------------------------------------
# --- Console
# --------------------------------------------------------------------
SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(SOURCES console.cpp console.h)
ADD_LIBRARY(console STATIC ${SOURCES})
ADD_DEPENDENCIES(console stdafx misc logs threads)

# --------------------------------------------------------------------
# --- GUI
# --------------------------------------------------------------------
SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(SOURCES gui.cpp gui.h)
ADD_LIBRARY(gui STATIC ${SOURCES})
ADD_DEPENDENCIES(gui stdafx gfx threads misc)

# --------------------------------------------------------------------
# --- Config
# --------------------------------------------------------------------
SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(SOURCES taconfig.cpp taconfig.h ta3dbase.h)
ADD_LIBRARY(taconfig STATIC ${SOURCES})
ADD_DEPENDENCIES(taconfig stdafx misc)

# --------------------------------------------------------------------
# --- File formats
# --------------------------------------------------------------------
SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(SOURCES gaf.cpp gaf.h
            3do.cpp 3do.h
            3ds.cpp 3ds.h)
ADD_LIBRARY(fileformats STATIC ${SOURCES})
ADD_DEPENDENCIES(fileformats stdafx particles i18n taconfig ingame)

# --------------------------------------------------------------------
# --- Audio
# --------------------------------------------------------------------
SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(SOURCES TA3D_Audio.cpp TA3D_Audio.h)
ADD_LIBRARY(audio STATIC ${SOURCES})
ADD_DEPENDENCIES(audio stdafx misc threads ingame)

# --------------------------------------------------------------------
# --- HPI
# --------------------------------------------------------------------
SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(SOURCES TA3D_hpi.cpp TA3D_hpi.h
            hpiview.cpp)
ADD_LIBRARY(hpi STATIC ${SOURCES})
ADD_DEPENDENCIES(hpi stdafx tajpeg misc)






# --------------------------------------------------------------------
# --- EDITOR
# --------------------------------------------------------------------


if( WIN32 OR CMAKE_CROSSCOMPILING )
	SET( 3DMEDITOR_BIN 3dmeditor )
else( WIN32 OR CMAKE_CROSSCOMPILING )
	SET( 3DMEDITOR_BIN 3dmeditor-bin )
endif( WIN32 OR CMAKE_CROSSCOMPILING )

ADD_EXECUTABLE(${3DMEDITOR_BIN} 3dmeditor.cpp ta3d.h 3dmeditor_sub.cpp 3dmeditor.h cTAFileParser.cpp cTAFileParser.h stdafx.h TA3D_NameSpace.cpp TA3D_NameSpace.h)

ADD_DEPENDENCIES(${3DMEDITOR_BIN} stdafx logs console debuginfos threads 3dtoolbox misc i18n tajpeg gfx fileformats particles audio hpi taconfig gui ingame)
target_link_libraries(${3DMEDITOR_BIN} stdafx logs console debuginfos threads 3dtoolbox misc i18n tajpeg gfx fileformats particles audio hpi taconfig gui ingame)
IF(APPLE)
    SET(GUI_TYPE MACOSX_BUNDLE)
    SET(MACOSX_BUNDLE_BUNDLE_NAME "TA3D Editor")
    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "ta3d-editor")
ENDIF(APPLE)



# --------------------------------------------------------------------
# --- TA3D
# --------------------------------------------------------------------
IF(APPLE)
	#SET(TA3D_BIN ta3d)
	SET(TA3D_BIN ta3d-bin)
ELSE(APPLE)
	SET(TA3D_BIN ta3d-bin)
ENDIF(APPLE)


SET(SOURCES ta3d.cpp ta3d.h main.cpp install.cpp cTA3D_Engine.cpp cTA3D_Engine.h
            cTAFileParser.cpp cTAFileParser.h
            restore.cpp restore.h
            menu.cpp menu.h
            EngineClass.cpp EngineClass.h
            UnitEngine.cpp UnitEngine.h
            fbi.cpp fbi.h intro.cpp intro.h 
            tdf.cpp tdf.h tnt.cpp tnt.h 
            icon.cpp stdafx.h 
            TA3D_NameSpace.cpp TA3D_NameSpace.h)
ADD_EXECUTABLE(${TA3D_BIN} ${SOURCES})

#IF(NOT APPLE)
#    ADD_EXECUTABLE(${TA3D_BIN} ${SOURCES})
#ELSE(NOT APPLE)
#
#    #
#    # Specific commands for OS X
#    #
#
#    # Icon
#    ADD_CUSTOM_COMMAND(OUTPUT "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Resources/ta3d.icns"
#        COMMAND  mkdir -p "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Resources" && cp -f
#        ARGS "${PROJECT_SOURCE_DIR}/pictures/ta3d.icns" "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Resources/ta3d.icns"
#        DEPENDS "${PROJECT_SOURCE_DIR}/pictures/ta3d.icns") 
#    SET(SOURCES ${SOURCES} "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Resources/ta3d.icns")
#
#    # FMOD
#    ADD_CUSTOM_COMMAND(OUTPUT "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Frameworks/libfmodex.dylib"
#        COMMAND  mkdir -p "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Frameworks" && cp -f
#        ARGS "${PROJECT_SOURCE_DIR}/src/tools/darwin/fmod/4.14.07/lib/libfmodex.dylib" "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Frameworks/libfmodex.dylib"
#        DEPENDS "${PROJECT_SOURCE_DIR}/src/tools/darwin/fmod/4.14.07/lib/libfmodex.dylib") 
#    SET(SOURCES ${SOURCES} "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Frameworks/libfmodex.dylib")
#
#    # LibZ
#    ADD_CUSTOM_COMMAND(OUTPUT "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Frameworks/libz.1.dylib"
#        COMMAND  mkdir -p "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Frameworks" && cp -fH
#        ARGS "/opt/local/lib/libz.1.dylib" "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Frameworks/libz.1.dylib"
#        DEPENDS "/opt/local/lib/libz.1.dylib") 
#    SET(SOURCES ${SOURCES} "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Frameworks/libz.1.dylib")
#
#    # Allegro
#    ADD_CUSTOM_COMMAND(OUTPUT "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Frameworks/liballeg-4.2.2.dylib"
#        COMMAND  mkdir -p "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Frameworks" && cp -fH
#        ARGS "/opt/local/lib/liballeg-4.2.2.dylib" "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Frameworks/liballeg-4.2.2.dylib"
#        DEPENDS "/opt/local/lib/liballeg-4.2.2.dylib") 
#    SET(SOURCES ${SOURCES} "${PROJECT_SOURCE_DIR}/ta3d.app/Contents/Frameworks/liballeg-4.2.2.dylib")
#
#
#    # OS X
#    ADD_EXECUTABLE(${TA3D_BIN} MACOSX_BUNDLE ${SOURCES})
#    SET(GUI_TYPE MACOSX_BUNDLE)
#    SET(MACOSX_BUNDLE_BUNDLE_NAME "TA3D")
#    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "ta3d")
#    SET(MACOSX_BUNDLE_ICON_FILE "ta3d.icns")
#
#    # Relink
#    ADD_CUSTOM_COMMAND(TARGET ${TA3D_BIN} POST_BUILD
#        COMMAND install_name_tool ARGS -change './libfmodex.dylib' @executable_path/../Frameworks/libfmodex.dylib ${PROJECT_SOURCE_DIR}/ta3d.app/Contents/MacOS/ta3d )
#    ADD_CUSTOM_COMMAND(TARGET ${TA3D_BIN} POST_BUILD
#        COMMAND install_name_tool ARGS -change '/opt/local/lib/libz.1.dylib' @executable_path/../Frameworks/libz.1.dylib ${PROJECT_SOURCE_DIR}/ta3d.app/Contents/MacOS/ta3d )
#    ADD_CUSTOM_COMMAND(TARGET ${TA3D_BIN} POST_BUILD
#        COMMAND install_name_tool ARGS -change '/opt/local/lib/liballeg-4.2.2.dylib' @executable_path/../Frameworks/liballeg-4.2.2.dylib ${PROJECT_SOURCE_DIR}/ta3d.app/Contents/MacOS/ta3d )
#
#ENDIF(NOT APPLE)

ADD_DEPENDENCIES(${TA3D_BIN} stdafx logs console debuginfos threads i18n 3dtoolbox misc ingame menus tajpeg gfx fileformats particles weapons ai audio hpi gui taconfig network scripts)
target_link_libraries(${TA3D_BIN} stdafx logs console debuginfos threads i18n 3dtoolbox misc ingame menus tajpeg gfx fileformats particles weapons ai audio hpi gui taconfig network scripts)



#
# --- External libraries
#

IF(WIN32)
    FIND_LIBRARY(VAR_LIB_FMODEX fmodex PATHS "${CMAKE_SOURCE_DIR}/src/tools/win32/libs" NO_SYSTEM_ENVIRONMENT_PATH)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${CMAKE_SOURCE_DIR}/src/tools/win32/fmod")
ELSE(WIN32) 
IF(APPLE)
    FIND_LIBRARY(VAR_LIB_FMODEX fmodex PATHS src/tools/darwin/fmod/4.14.07/lib/ NO_SYSTEM_ENVIRONMENT_PATH)
    IF(NOT VAR_LIB_FMODEX)
        MESSAGE(FATAL_ERROR "FMOD is required")
    ENDIF(NOT VAR_LIB_FMODEX)
ELSE(APPLE)
    FIND_LIBRARY( VAR_LIB_FMODEX   fmodex)
    FIND_LIBRARY( VAR_LIB_FMODEX64 fmodex64)
ENDIF(APPLE)
ENDIF(WIN32) 
    
IF( VAR_LIB_FMODEX )
   target_link_libraries(${TA3D_BIN} fmodex)
   target_link_libraries(${3DMEDITOR_BIN} fmodex)
elseif( VAR_LIB_FMODEX64 )
   target_link_libraries(${TA3D_BIN} fmodex64)
   target_link_libraries(${3DMEDITOR_BIN} fmodex64)
else( VAR_LIB_FMODEX )
   message( FATAL_ERROR "FMODex is required, please install FMODex" )
endif( VAR_LIB_FMODEX )

IF(WIN32)
    target_link_libraries(${3DMEDITOR_BIN} agl lua gdi32 alleg opengl32 glu32)
    target_link_libraries(${TA3D_BIN} agl lua gdi32 alleg opengl32 glu32 NL)
ENDIF(WIN32)
