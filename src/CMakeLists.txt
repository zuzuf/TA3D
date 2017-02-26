PROJECT(TA3DRepository)
cmake_minimum_required(VERSION 3.3)

Message(STATUS "")
Message(STATUS "--- TA3D, Remake of the Total Annihilation engine ---")
Message(STATUS "")


SET(TA3D_WEBSITE                "http://www.ta3d.org")
SET(TA3D_WEBSITE_FORUM          "http://www.ta3d.org/forums/")
SET(TA3D_MAIL_BUGREPORT         "dev@ta3d.org")
SET(TA3D_WEBSITE_NEW_BUGREPORT  "http://trac.ta3d.org/newticket")

SET(TA3D_VERSION_HI     "0")     # 0.7.0
SET(TA3D_VERSION_LO     "7")
SET(TA3D_VERSION_BUILD  "0")
SET(TA3D_VERSION_TAG    "WIP")


IF (NOT CMAKE_BUILD_TYPE)
        SET(CMAKE_BUILD_TYPE "Debug")
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



MESSAGE(STATUS "Version : v${TA3D_VERSION_HI}.${TA3D_VERSION_LO}.${TA3D_VERSION_BUILD} (Rev: ${SVN_REV})")

# Information about the current Operating System
Message(STATUS " System : ${CMAKE_SYSTEM} (${CMAKE_SYSTEM_PROCESSOR})")

# FTGL
IF(NOT APPLE)
	ADD_SUBDIRECTORY(libs/ftgl)
ENDIF(NOT APPLE)

# TA3D
ADD_SUBDIRECTORY(ta3d)

