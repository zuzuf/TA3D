cmake_minimum_required(VERSION 2.8)
include("../cmake/message.cmake")


YMESSAGE("")
YMESSAGE("Yuni Framework - Samples")
YMESSAGE("")
project(YUNI-SAMPLES CXX)

if(POLICY CMP0011)
	cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)


# To find 'FindYuni.cmake'
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../packages/cmake")

include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/CommonSettings.cmake")

add_custom_target(samples)

set(Yuni_YUNICONFIG_PATH ${YUNI_CONFIG})
find_package(Yuni COMPONENTS core)
if(Yuni_FOUND)

	# Infos
	YMESSAGE("The Yuni Framework : ${Yuni_INFOS}")

	add_subdirectory(audio)
	add_subdirectory(core)
	add_subdirectory(gfx)
	add_subdirectory(devices)
	add_subdirectory(jobs)
	add_subdirectory(script)
	add_subdirectory(threads)
	add_subdirectory(extra)
else()
	YMESSAGE("A working Yuni Framework was not found (could not locate yuni-config).")
	YMESSAGE("If you are not building from within the source tree, you should define")
	YMESSAGE("YUNI_CONFIG to the path to the yuni-config binary, e.g.:")
	YMESSAGE(" $ cmake -DYUNI_CONFIG=path/to/yuni-config <samples_path>")
	YERROR("Aborting.")

endif()

