

find_package(Yuni COMPONENTS core ui)
if(Yuni_FOUND)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Yuni_CXXFLAGS}")
	link_libraries("${Yuni_LIBS}")

	message(STATUS "Sample: Gfx / Display a simple window")
	add_executable(gfx_00_window main.cpp)
endif(Yuni_FOUND)

