IF(NOT YUNI_AUTO_COMPILE_YUNI_CONFIG)

	Project(yuni-config CXX)
	CMake_minimum_required(VERSION 2.8)

	IF(POLICY CMP0005)
		CMake_Policy(SET CMP0005 NEW)
	endif(POLICY CMP0005)
	IF(POLICY CMP0011)
		CMake_Policy(SET CMP0011 NEW)
	endif(POLICY CMP0011)

	Include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/message.cmake")

endif(NOT YUNI_AUTO_COMPILE_YUNI_CONFIG)

if(YUNI_AUTO_COMPILE_YUNI_CONFIG)
	YMESSAGE("")
	YMESSAGE("yuni-config  (self)")
	YMESSAGE("")
else()
	YMESSAGE("")
	YMESSAGE("yuni-config")
	YMESSAGE("")
endif(YUNI_AUTO_COMPILE_YUNI_CONFIG)


include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/CommonSettings.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/Modules.cmake")


include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../..")

# Disable all code related to thread-safety
add_definitions("-DYUNI_NO_THREAD_SAFE")

# Add definitions to help finding yuni parameters once installed (prefix + versioned path)
add_definitions("-DYUNI_VERSIONED_INST_PATH=\"${YUNI_VERSIONED_INST_PATH}\"")
add_definitions("-DYUNI_INSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
add_definitions("-DYUNI_SOURCE_TREE=\"${YUNI_SOURCE_TREE}\"")



set(src
	../../yuni/core/version/version.cpp
	../../yuni/core/getopt/option.cpp ../../yuni/core/getopt/parser.cpp
	../../yuni/core/io/io.cpp
	../../yuni/core/io/file/file.cpp
	../../yuni/core/io/file/openmode.cpp
	../../yuni/core/io/file/stream.cpp
	../../yuni/core/io/exists.cpp
	../../yuni/core/io/directory/current.cpp
	../../yuni/core/customstring/traits/traits.cpp

	# iterator
	../../yuni/core/io/directory/iterator.h
	../../yuni/core/io/directory/iterator/iterator.h
	../../yuni/core/io/directory/iterator/iterator.hxx
	../../yuni/core/io/directory/iterator/iterator.cpp
	../../yuni/core/io/directory/iterator/detachedthread.h

	versions.h versions.hxx versions.cpp
	program.h program.hxx program.cpp
	main.cpp)

# Build the in-tree version of yuni-config.
add_executable(yuni-config ${src})
set_target_properties(yuni-config PROPERTIES
	COMPILE_FLAGS "-DYUNI_YUNICONFIG_IN_TREE=\\\"${CMAKE_CURRENT_SOURCE_DIR}\\\""
	RUNTIME_OUTPUT_DIRECTORY "${YUNI_OUTPUT_DIRECTORY}/bin"
)

# If we are not bootstrapping, also build the "installable" yuni-config.
IF(NOT YUNI_BOOTSTRAP_YUNI_CONFIG)

	add_executable(yuni-config-${YUNI_VERSION} ${src})
	set_target_properties(yuni-config-${YUNI_VERSION} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${YUNI_OUTPUT_DIRECTORY}/bin")
	install(TARGETS yuni-config-${YUNI_VERSION} RUNTIME DESTINATION bin)

endif(NOT YUNI_BOOTSTRAP_YUNI_CONFIG)

