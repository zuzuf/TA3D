ADD_SUBDIRECTORY(logs)
ADD_SUBDIRECTORY(threads)
ADD_SUBDIRECTORY(misc)
ADD_SUBDIRECTORY(jpeg)
ADD_SUBDIRECTORY(gfx)
ADD_SUBDIRECTORY(network)
ADD_SUBDIRECTORY(ai)


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
# --- i18n
# --------------------------------------------------------------------
SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(SOURCES i18n.cpp i18n.h)
ADD_LIBRARY(i18n STATIC ${SOURCES})
ADD_DEPENDENCIES(i18n stdafx misc)

# --------------------------------------------------------------------
# --- GUI
# --------------------------------------------------------------------
SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(SOURCES gui.cpp gui.h)
ADD_LIBRARY(gui STATIC ${SOURCES})
ADD_DEPENDENCIES(gui stdafx gfx threads misc)

# --------------------------------------------------------------------
# --- File formats
# --------------------------------------------------------------------
SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(SOURCES gaf.cpp gaf.h
            3do.cpp 3do.h
            3ds.cpp 3ds.h)
ADD_LIBRARY(fileformats STATIC ${SOURCES})
ADD_DEPENDENCIES(fileformats stdafx particles taconfig)

# --------------------------------------------------------------------
# --- Audio
# --------------------------------------------------------------------
SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(SOURCES TA3D_Audio.cpp TA3D_Audio.h)
ADD_LIBRARY(audio STATIC ${SOURCES})
ADD_DEPENDENCIES(audio stdafx misc threads)

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
# --- Config
# --------------------------------------------------------------------
SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(SOURCES taconfig.cpp taconfig.h matrix.h ta3dbase.h)
ADD_LIBRARY(config STATIC ${SOURCES})
ADD_DEPENDENCIES(config stdafx)






# --------------------------------------------------------------------
# --- EDITOR
# --------------------------------------------------------------------


if( WIN32 OR CMAKE_CROSSCOMPILING )
	SET( 3DMEDITOR 3dmeditor )
else( WIN32 OR CMAKE_CROSSCOMPILING )
	SET( 3DMEDITOR 3dmeditor-bin )
endif( WIN32 OR CMAKE_CROSSCOMPILING )

add_executable(${3DMEDITOR} 3dmeditor.cpp ta3d.h 3dmeditor_sub.cpp 3dmeditor.h cTAFileParser.cpp cTAFileParser.h stdafx.h TA3D_NameSpace.cpp TA3D_NameSpace.h TA3D_Platform.h matrix.h ta3dbase.h)

ADD_DEPENDENCIES(${3DMEDITOR} stdafx logs console debuginfos threads misc i18n tajpeg gfx fileformats particles audio hpi config gui)
target_link_libraries(${3DMEDITOR} stdafx logs console debuginfos threads misc i18n tajpeg gfx fileformats particles audio hpi config gui)
IF(APPLE)
    SET(GUI_TYPE MACOSX_BUNDLE)
    SET(MACOSX_BUNDLE_BUNDLE_NAME "TA3D Editor")
    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "ta3d-editor")
ENDIF(APPLE)



# --------------------------------------------------------------------
# --- TA3D
# --------------------------------------------------------------------

add_executable(ta3d-bin ta3d.cpp ta3d.h main.cpp install.cpp cTA3D_Engine.cpp cTA3D_Engine.h cTAFileParser.cpp cTAFileParser.h restore.cpp restore.h menu.cpp menu.h cob.cpp cob.h EngineClass.cpp EngineClass.h UnitEngine.cpp UnitEngine.h fbi.cpp fbi.h intro.cpp intro.h tdf.cpp tdf.h tnt.cpp tnt.h weapons.cpp weapons.h fx.cpp fx.h script.cpp script.h icon.cpp stdafx.h TA3D_NameSpace.cpp TA3D_NameSpace.h superqueue.cpp superqueue.h TA3D_Platform.h matrix.h ta3dbase.h)

ADD_DEPENDENCIES(ta3d-bin stdafx logs console debuginfos threads i18n misc tajpeg gfx fileformats particles ai audio hpi gui config network)
target_link_libraries(ta3d-bin stdafx logs console debuginfos threads i18n misc tajpeg gfx fileformats particles ai audio hpi gui config network)
IF(APPLE)
    SET(GUI_TYPE MACOSX_BUNDLE)
    SET(MACOSX_BUNDLE_BUNDLE_NAME "TA3D")
    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "ta3d")
ENDIF(APPLE)




#
# --- External libraries
#

IF(WIN32 AND NOT CMAKE_CROSSCOMPILING)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${CMAKE_SOURCE_DIR}/tools/win32/fmod")
ELSE(WIN32 AND NOT CMAKE_CROSSCOMPILING)
    
    IF(APPLE)
        FIND_LIBRARY(VAR_LIB_FMODEX fmodex PATHS src/tools/darwin/fmod/4.14.07/lib/ NO_SYSTEM_ENVIRONMENT_PATH)
        IF(NOT VAR_LIB_FMODEX)
            MESSAGE(FATAL_ERROR "FMOD is required")
        ENDIF(NOT VAR_LIB_FMODEX)
    ELSE(APPLE)
        FIND_LIBRARY( VAR_LIB_FMODEX   fmodex)
        FIND_LIBRARY( VAR_LIB_FMODEX64 fmodex64)
    ENDIF(APPLE)
    
    IF( VAR_LIB_FMODEX )
        target_link_libraries(ta3d-bin fmodex)
        target_link_libraries(${3DMEDITOR} fmodex)
    else( VAR_LIB_FMODEX64 )
        target_link_libraries(ta3d-bin fmodex64)
        target_link_libraries(${3DMEDITOR} fmodex64)
    else( VAR_LIB_FMODEX )
        message( FATAL_ERROR "FMODex is required, please install FMODex" )
    endif( VAR_LIB_FMODEX )
ENDIF(WIN32 AND NOT CMAKE_CROSSCOMPILING)

