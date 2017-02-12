
#
# CMake Stuff
#
#set(CMAKE_VERBOSE_MAKEFILE ON)
# Policy
if(NOT CMAKE_MINOR_VERSION EQUAL 4 OR NOT CMAKE_MAJOR_VERSION EQUAL 2 )
	cmake_policy(SET CMP0004 OLD)
	cmake_policy(SET CMP0003 NEW)
endif()
include(CheckIncludeFile)

#
# Getting the folder where this file is located
#
set(CurrentFolder "${CMAKE_CURRENT_LIST_FILE}")
string(REPLACE "\\CommonSettings.cmake" "" CurrentFolder "${CurrentFolder}")
string(REPLACE "/CommonSettings.cmake" "" CurrentFolder "${CurrentFolder}")
# Current Folder : ${CurrentFolder}

#
# Detect Special Instructions Set (SSE, MMX...)
#
include("${CurrentFolder}/DetectInstructionsSets.cmake")



set(YUNI_COMMON_GCC_OPTIONS  "-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -Woverloaded-virtual -Wall -Wextra -Wunused-parameter -Wconversion")
set(YUNI_COMMON_GCC_OPTIONS  "${YUNI_COMMON_GCC_OPTIONS} -Wmissing-noreturn -Wcast-align  -Wfloat-equal -Wundef")

include(CheckCXXCompilerFlag)
if(MINGW OR (NOT WIN32 AND NOT WIN64))
	check_cxx_compiler_flag("-std=c++0x" YUNI_HAS_GCC_CPP0X_SUPPORT)
	if(YUNI_HAS_GCC_CPP0X_SUPPORT)
		set(YUNI_COMMON_GCC_OPTIONS  "${YUNI_COMMON_GCC_OPTIONS} -std=c++0x")
	endif()
endif()


if(NOT WIN32)
	set(CMAKE_CXX_FLAGS_RELEASE         "${YUNI_COMMON_GCC_OPTIONS} -O3 -fomit-frame-pointer -fstrict-aliasing -momit-leaf-frame-pointer -fno-tree-pre -falign-loops -mfpmath=sse -msse -msse2 -Wuninitialized")
	set(CMAKE_CXX_FLAGS_DEBUG           "${YUNI_COMMON_GCC_OPTIONS} -g -ggdb3")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "${YUNI_COMMON_GCC_OPTIONS} -O3 -fomit-frame-pointer -mfpmath=sse -msse -msse2")
endif()
if(MINGW)
	set(CMAKE_CXX_FLAGS_RELEASE         "${YUNI_COMMON_GCC_OPTIONS} -O3 -fomit-frame-pointer -fstrict-aliasing -momit-leaf-frame-pointer -fno-tree-pre -falign-loops -mthreads -Wuninitialized")
	set(CMAKE_CXX_FLAGS_DEBUG           "${YUNI_COMMON_GCC_OPTIONS} -mthreads -g2")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "${YUNI_COMMON_GCC_OPTIONS} -mthreads -g2")
endif()
if(MSVC)
	set(CMAKE_CXX_FLAGS_RELEASE         "/EHsc /Ox /Ob2 /Ot /O2 /Oy /MD /GS- /Gy /arch:SSE2")
	set(CMAKE_CXX_FLAGS_DEBUG           "/EHsc /GR /Ob1 /Ot /MDd /fp:except /W3")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "/EHsc /Ox /Ob2 /Ot /O2 /Oy /MDd /GS- /Gy")
endif()
if(APPLE)
	# The SSE instruction set is mandatory
	set(CMAKE_CXX_FLAGS_DEBUG           "${CMAKE_CXX_FLAGS_DEBUG}   -msse2 -gfull -fvisibility=hidden")
	set(CMAKE_CXX_FLAGS_RELEASE         "${CMAKE_CXX_FLAGS_RELEASE} -msse2 -fvisibility=hidden")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "${CMAKE_CXX_FLAGS_RELEASE} -msse2 -fvisibility=hidden")
endif()

set(CMAKE_CXX_FLAGS_RELEASE       "${CMAKE_CXX_FLAGS_RELEASE} ${YUNI_PROFILE_CXX_FLAGS_INSTRUCTIONS_SETS}")
set(CMAKE_CXX_FLAGS_DEBUG         "${CMAKE_CXX_FLAGS_DEBUG} ${YUNI_PROFILE_CXX_FLAGS_INSTRUCTIONS_SETS}")
set(CMAKE_CXX_FLAGS_RELWITHDEBUG  "${CMAKE_CXX_FLAGS_RELWITHDEBUG} ${YUNI_PROFILE_CXX_FLAGS_INSTRUCTIONS_SETS}")

# Override
if (NOT "${YUNI_CXX_FLAGS_OVERRIDE}" STREQUAL "")
	set(CMAKE_CXX_FLAGS_DEBUG            "${YUNI_CXX_FLAGS_OVERRIDE}")
	set(CMAKE_CXX_FLAGS_RELEASE          "${YUNI_CXX_FLAGS_OVERRIDE}")
	set(CMAKE_CXX_FLAGS_RELWITHDEBUG     "${YUNI_CXX_FLAGS_OVERRIDE}")
	set(CMAKE_CXX_FLAGS_RELWITHDEBUGINFO "${YUNI_CXX_FLAGS_OVERRIDE}")
endif()

if(NOT "${YUNI_CXX_FLAGS_OVERRIDE_ADD_DEBUG}" STREQUAL "")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${YUNI_CXX_FLAGS_OVERRIDE_ADD_DEBUG}")
endif()
if(NOT "${YUNI_CXX_FLAGS_OVERRIDE_ADD_RELEASE}" STREQUAL "")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${YUNI_CXX_FLAGS_OVERRIDE_ADD_RELEASE}")
endif()
if(NOT "${YUNI_CXX_FLAGS_OVERRIDE_ADD_RELWITHDEBINFO}" STREQUAL "")
		set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${YUNI_CXX_FLAGS_OVERRIDE_ADD_RELWITHDEBINFO}")
endif()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "release" OR "${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
endif()


#
# Extra - Mac OS X Bundles
#
if(APPLE)
	set(MACOSX_BUNDLE_COPYRIGHT "Yuni Framework - 2011")
	if(YUNI_MACOX_UNIVERSAL_BINARIES)
		YMESSAGE("Enabled universal binaries (custom : ${YUNI_MACOX_UNIVERSAL_BINARIES})")
		set(CMAKE_OSX_ARCHITECTURES "${YUNI_MACOX_UNIVERSAL_BINARIES}") # ppc;i386;ppc64;x86_64
	else()
		YMESSAGE("Enabled universal binaries (i386, x86_64)")
		set(CMAKE_OSX_ARCHITECTURES "i686;x86_64") # ppc;i386;ppc64;x86_64
	endif()
endif()


