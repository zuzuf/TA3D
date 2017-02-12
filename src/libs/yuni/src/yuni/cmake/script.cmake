
YMESSAGE(":: [Module] Script")

Include(CheckIncludeFile)
LIBYUNI_CONFIG_LIB("both" "script"     "yuni-static-script")


# Scripts
set(SRC_SCRIPT
			script/script.h script/script.hxx script/script.cpp
			script/private.h script/private.hxx
			script/args.hxx
			private/script/script.defines.h private/script/script.undefs.h
	)
source_group("Script\\Script Abstraction" FILES ${SRC_SCRIPT})



# External Lib: Lua script
if(YUNI_EXTERNAL_SCRIPT_LUA)
	YMESSAGE(":: [Module] Script::Lua")

	if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/cmake/deps/lua-${YUNI_DvP_LUA_MODE}.cmake)
		include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/deps/lua-${YUNI_DvP_LUA_MODE}.cmake)
	else()
		YFATAL("[!!] Invalid YUNI_DvP_LUA_MODE: ${YUNI_DvP_LUA_MODE}")
	endif()

	# Check if we really have a lua.h
	set(CMAKE_REQUIRED_INCLUDES "${YUNI_EXT_LUA_INCLUDE}")
	check_include_files("lua.h" YUNI_HAS_LUA_H)

	if(NOT YUNI_HAS_LUA_H)
		set(YUNI_CMAKE_ERROR 1)
		YMESSAGE(    "[!!] Impossible to find lua.h. Please check your profile.")
		YMESSAGE(    " * Packages needed on Debian: liblua5.1-dev")
	endif()

	# Add any detected extra header path
	# We do not expose Lua, so there's nothing to add to yuni-config
	if(YUNI_EXT_LUA_INCLUDE)
		include_directories(${YUNI_EXT_LUA_INCLUDE})
	endif()

	# Lua is dependent on Script module
	LIBYUNI_CONFIG_DEPENDENCY("lua" "script")
	
	set(SRC_EXTERNAL_SCRIPT_LUA
			# We can build the devpack source files here.
			${YUNI_EXT_LUA_SRC}
			script/lua/lua.h script/lua/lua.cpp script/lua/args.cpp
			private/script/lua.proxy.h
		)
	source_group("Script\\Script Lua language support" FILES ${SRC_EXTERNAL_SCRIPT_LUA})

endif()


add_library(yuni-static-script STATIC
	${SRC_SCRIPT} ${SRC_EXTERNAL_SCRIPT_LUA})

# Setting output path
set_target_properties(yuni-static-script PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${YUNI_OUTPUT_DIRECTORY}/lib")

# Installation
install(TARGETS yuni-static-script ARCHIVE DESTINATION lib/${YUNI_VERSIONED_INST_PATH})

# Install Script-related headers
install(
	DIRECTORY script
	DESTINATION include/${YUNI_VERSIONED_INST_PATH}/yuni
	FILES_MATCHING
		PATTERN "*.h"
		PATTERN "*.hxx"
	PATTERN ".svn" EXCLUDE
	PATTERN "CMakeFiles" EXCLUDE
	PATTERN "cmake" EXCLUDE
)

# Link
target_link_libraries(yuni-static-script yuni-static-core)
