PROJECT(TA3DRepository)
cmake_minimum_required(VERSION 2.6)

Message(STATUS "")
Message(STATUS "--- TA3D, Remake of the Total Annihilation engine ---")
Message(STATUS "")


SET(TA3D_WEBSITE                "http://www.ta3d.org")
SET(TA3D_WEBSITE_FORUM          "http://www.ta3d.org/forums/")
SET(TA3D_MAIL_BUGREPORT         "dev@ta3d.org")
SET(TA3D_WEBSITE_NEW_BUGREPORT  "http://trac.ta3d.org/newticket")

SET(TA3D_VERSION_HI     "0")     # 0.6.0
SET(TA3D_VERSION_LO     "6")
SET(TA3D_VERSION_PATCH  "0")
SET(TA3D_VERSION_TAG    "WIP")


#
# Getting the latest revision number
#
IF(WIN32 AND NOT CMAKE_CROSSCOMPILING)
	EXECUTE_PROCESS(COMMAND "subwcrev" "${PROJECT_SOURCE_DIR}" OUTPUT_VARIABLE SVN_REV)
	STRING(REGEX REPLACE ".* committed at revision ([0-9]+).*" "\\1" SVN_REV "${SVN_REV}")
ELSE(WIN32 AND NOT CMAKE_CROSSCOMPILING)
	EXECUTE_PROCESS(COMMAND "svnversion" "-n" "${PROJECT_SOURCE_DIR}" OUTPUT_VARIABLE SVN_REV)
	STRING(REGEX REPLACE "[0-9]+:" "" SVN_REV "${SVN_REV}")
	STRING(REPLACE "M" "" SVN_REV "${SVN_REV}")
ENDIF(WIN32 AND NOT CMAKE_CROSSCOMPILING)
MESSAGE(STATUS "Version : v${TA3D_VERSION_HI}.${TA3D_VERSION_LO}.${TA3D_VERSION_PATCH} (Rev: ${SVN_REV})")

# Information about the current Operating System
Message(STATUS " System : ${CMAKE_SYSTEM} (${CMAKE_SYSTEM_PROCESSOR})")


#
# Temporary Fix for a Yuni workaround
#
# Yuni does not provide a way to get all compilation flags.
# It is especially annoying on Windows with MinGW where PThread is missing.
# This must be removed as soon as possible.
#
IF(WIN32)
	include_directories("${CMAKE_CURRENT_SOURCE_DIR}/libs/yuni/devpacks/pthreads/2.8.0/r3/i386/mingw/include")
	SET(YUNI_TEMP_FIX_STATICLIB_PTHREAD "${CMAKE_CURRENT_SOURCE_DIR}/libs/yuni/devpacks/pthreads/2.8.0/r3/i386/mingw/lib/static/libpthreadGC2.a")
	LINK_LIBRARIES(${YUNI_TEMP_FIX_STATICLIB_PTHREAD})
ENDIF(WIN32)

# Yuni
SET(YUNI_STATIC_CORE "yuni-static-core")
SET(YUNI_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/libs/yuni/src")
ADD_SUBDIRECTORY(libs/yuni/src)

# FTGL
IF(NOT APPLE)
	ADD_SUBDIRECTORY(libs/ftgl)
ENDIF(NOT APPLE)

# TA3D
ADD_SUBDIRECTORY(ta3d)

