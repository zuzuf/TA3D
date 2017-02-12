
find_package(Yuni COMPONENTS core devices display)
if(Yuni_FOUND)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Yuni_CXXFLAGS}")
	link_libraries("${Yuni_LIBS}")

	message(STATUS "Sample: Devices / List all monitors")
	add_executable(devices_00_monitors main.cpp)
endif(Yuni_FOUND)

