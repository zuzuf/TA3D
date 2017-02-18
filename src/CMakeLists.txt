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
SET(TA3D_VERSION_TAG    "")


IF (NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE "debug")
ENDIF()


IF (APPLE)
	SET(CMAKE_OSX_ARCHITECTURES "i386;x86_64")
ENDIF()

#
# Getting the latest revision number
#
IF(WIN32 AND NOT CMAKE_CROSSCOMPILING)
	EXECUTE_PROCESS(COMMAND "subwcrev.exe" "${PROJECT_SOURCE_DIR}" OUTPUT_VARIABLE SVN_REV)
	STRING(REGEX REPLACE ".* committed at revision ([0-9]+).*" "\\1" SVN_REV "${SVN_REV}")
ELSE(WIN32 AND NOT CMAKE_CROSSCOMPILING)
	EXECUTE_PROCESS(COMMAND "svnversion" "-n" "${PROJECT_SOURCE_DIR}" OUTPUT_VARIABLE SVN_REV)
	STRING(REGEX REPLACE "[0-9]+:" "" SVN_REV "${SVN_REV}")
	STRING(REPLACE "M" "" SVN_REV "${SVN_REV}")
ENDIF(WIN32 AND NOT CMAKE_CROSSCOMPILING)

# Dealing with bad revision numbers
IF(NOT "${SVN_REV}" GREATER 1)
	SET(SVN_REV 1)
ENDIF(NOT "${SVN_REV}" GREATER 1)



MESSAGE(STATUS "Version : v${TA3D_VERSION_HI}.${TA3D_VERSION_LO}.${TA3D_VERSION_PATCH} (Rev: ${SVN_REV})")

# Information about the current Operating System
Message(STATUS " System : ${CMAKE_SYSTEM} (${CMAKE_SYSTEM_PROCESSOR})")



#
# Yuni
#
# We have to copy the yuni's profile dedicated to Ta3D into the yuni sources folder.
#
CONFIGURE_FILE(
	"${CMAKE_CURRENT_SOURCE_DIR}/YuniProfileBuildTemplate.cmake"
	"${CMAKE_CURRENT_SOURCE_DIR}/libs/yuni/src/ProfileBuild.cmake" COPYONLY)
# The sources
ADD_SUBDIRECTORY(libs/yuni/src)
# At this point, the program 'yuni-config' should be available, as requested
# by the profile.
# We have to provide the path where the file 'FindYuni.cmake' is located
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs/yuni/packages/cmake")
# Trying to find the Yuni framework
Find_Package(Yuni COMPONENTS core)
IF(Yuni_FOUND)
	# Infos
	MESSAGE(STATUS "The Yuni Framework : ${Yuni_INFOS}")
ELSE(Yuni_FOUND)
	Message(SEND_ERROR "The Yuni Framework could not be found.")
ENDIF(Yuni_FOUND)



# FTGL
IF(NOT APPLE)
	ADD_SUBDIRECTORY(libs/ftgl)
ENDIF(NOT APPLE)

# SDL_sgui
IF (WIN32 OR (CMAKE_CROSSCOMPILING AND NOT UNIX))
#	SET( SDL_FOUND included )
#	SET( SDL_INCLUDE_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/ta3d/src/tools/win32/mingw32/include/SDL")
#	SET( SDL_LIBRARY "mingw32;${CMAKE_CURRENT_SOURCE_DIR}/ta3d/src/tools/win32/mingw32/libs/libSDLmain.a;${CMAKE_CURRENT_SOURCE_DIR}/src/tools/win32/mingw32/libs/SDL.dll")
#	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ta3d/src/tools/win32/mingw32/include)
ENDIF (WIN32 OR (CMAKE_CROSSCOMPILING AND NOT UNIX))
ADD_SUBDIRECTORY(libs/SDL_sgui)

# TA3D
ADD_SUBDIRECTORY(ta3d)

