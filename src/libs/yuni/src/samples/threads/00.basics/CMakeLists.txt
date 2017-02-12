

find_package(Yuni COMPONENTS core)
if(Yuni_FOUND)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Yuni_CXXFLAGS}")
	link_libraries("${Yuni_LIBS}")

	message(STATUS "Sample: Multi-Threading / A Simple custom thread")
	add_executable(threads_00_basics main.cpp)
endif(Yuni_FOUND)

