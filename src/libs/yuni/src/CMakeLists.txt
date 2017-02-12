
# Minimum CMake version check
cmake_minimum_required(VERSION 2.8)
message(STATUS "${CMAKE_CXX_FLAGS}")
include("cmake/message.cmake")

YMESSAGE("")
YMESSAGE("The Yuni Framework")
YMESSAGE("")
project(Yuni-Framework) 


# Package informations
set(YUNI_URL_WEBSITE             "http://www.libyuni.org")
set(YUNI_MAILING_LIST            "dev@libyuni.org")
# Version
set(YUNI_VERSION_HI              0)
set(YUNI_VERSION_LO              1)
set(YUNI_VERSION                 "${YUNI_VERSION_HI}.${YUNI_VERSION_LO}")
set(YUNI_VERSION_REV             0) # Svn Revision - see below
set(YUNI_TARGET                  "debug") # Target : debug/release - see Target.cmake
set(YUNI_URL_DEVPACK_REPOSITORY  "http://devpacks.libyuni.org/")
set(YUNI_URL_DEVPACK_SOURCE      "http://devpacks.libyuni.org/downloads")








#
# -----------------------------------------------------------------------------
#

# CMake special variables
set(VS_KEYWORD                   "YuniVS${YUNI_VERSION}")
set(VERSION                      "${YUNI_VERSION}")
set(VS_SCC_PROJECTNAME           "Yuni Framework")
set(PROJECT_LABEL                "Yuni Framework")


#
# CMake policies
#
# Preprocessor definition values are now escaped automatically
if(POLICY CMP0005)
	CMake_Policy(SET CMP0005 NEW)
endif()
if(POLICY CMP0011)
	CMake_Policy(SET CMP0011 NEW)
endif()


set(YUNI_INSTALL_CXX_FLAGS   "")
set(YUNI_INSTALL_CXX_DEFINES "")

# ICC Detection
# CMake does not handle ICC at all, mainly because ICC behaves like Visual Studio and
# it seems there is no reliable way to make the distinction
if (WIN32 OR WIN64)
	if (NOT MSVC AND NOT MINGW AND NOT CYGWIN)
		# Should behaves like Visual Studio, like ICC
		set(MSVC 1)
	endif()
endif()


# Target
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Target.cmake")
# Getting the latest revision number
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/SvnRevision.cmake")
# Informations about the system
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/SystemInformation.cmake")


#
# The target directory for static libs
# Various output directories
#
set(YUNI_VERSIONED_INST_PATH "yuni/${YUNI_VERSION}")
set(YUNI_OUTPUT_DIRECTORY    "${CMAKE_CURRENT_BINARY_DIR}/build/${YUNI_TARGET}/")
set(YUNI_SOURCE_TREE         "${CMAKE_CURRENT_SOURCE_DIR}/yuni/")

YMESSAGE("Folders")
YMESSAGE("         Source: ${CMAKE_CURRENT_SOURCE_DIR}")
YMESSAGE("     Build into: ${CMAKE_CURRENT_BINARY_DIR}")


# Custom settings
if("${PROFILE}" STREQUAL "")

	if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/ProfileBuild.cmake")
		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ProfileBuild.template.cmake
			${CMAKE_CURRENT_BINARY_DIR}/ProfileBuild.cmake)
		YMESSAGE("")
		YMESSAGE("A default profile has been created (`<build_directory>/ProfileBuild.cmake`)")
		YMESSAGE("Please edit this file to override the default settings")
		YMESSAGE("")
	endif()

	set(YUNI_PROFILE_PATH "${CMAKE_CURRENT_BINARY_DIR}/ProfileBuild.cmake")

else()

	if(NOT EXISTS "${PROFILE}")
		YERROR("Could not find the specified profile file: (`${PROFILE}`)")
	endif()

	set(YUNI_PROFILE_PATH "${PROFILE}")

endif()

include("${YUNI_PROFILE_PATH}")
YMESSAGE("  Using profile: ${YUNI_PROFILE_NAME}")
YMESSAGE("       found in: ${YUNI_PROFILE_PATH}")

# (Re)Check the target
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Target.cmake")

# DevPacks utilities
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/DevPack.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/DevPackStandardImports.cmake")

# Modules
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules.cmake")



# Sub Directories
add_subdirectory(yuni)

# tools (especially for yuni-config)
add_subdirectory(tools)



#
# Compiling yuni-config if required
#
if(YUNI_AUTO_COMPILE_YUNI_CONFIG)
	YMESSAGE("[yuni-config] Bootstraping")
	if(YUNI_TESTS)
		YMESSAGE("The bootstrapping is required to compile the testsuite")
	endif()
	# --
	YMESSAGE("Configuring yuni-config...")
	YMESSAGE("(in ${CMAKE_CURRENT_SOURCE_DIR}/tools/yuni-config)")
	# Create the yuni-config bootstrap directory.
	execute_process(COMMAND "${CMAKE_COMMAND}" -E make_directory
		"${CMAKE_CURRENT_BINARY_DIR}/tools/yuni-config-bootstrap"
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
	# Configure yuni-config like us.
	execute_process(COMMAND "${CMAKE_COMMAND}"
			${CMAKE_CURRENT_SOURCE_DIR}/tools/yuni-config
			"-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
			"-DYUNI_OUTPUT_DIRECTORY=${YUNI_OUTPUT_DIRECTORY}"
			"-DYUNI_BOOTSTRAP_YUNI_CONFIG=1"
			-G "${CMAKE_GENERATOR}"
			WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tools/yuni-config-bootstrap")
	# Compile it.
	YMESSAGE("Compiling yuni-config...")
	execute_process(COMMAND "${CMAKE_COMMAND}" --build
                              "${CMAKE_CURRENT_BINARY_DIR}/tools/yuni-config-bootstrap"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tools/yuni-config-bootstrap")
endif()

