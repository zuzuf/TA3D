


find_package(Yuni COMPONENTS core lua)
if(Yuni_FOUND)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Yuni_CXXFLAGS}")
	link_libraries("${Yuni_LIBS}")


	message(STATUS "Sample: Script / A Simple Lua Hello World (not functional yet)")
	add_executable(script_00_helloworld main.cpp "${CMAKE_CURRENT_BINARY_DIR}/helloworld.lua")
	add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/helloworld.lua"
	    COMMENT "Copying sample script"
		COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/helloworld.lua"
		 "${CMAKE_CURRENT_BINARY_DIR}/helloworld.lua"
	)
endif(Yuni_FOUND)

