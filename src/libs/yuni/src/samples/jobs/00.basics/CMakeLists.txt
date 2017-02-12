

find_package(Yuni COMPONENTS core)
if(Yuni_FOUND)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Yuni_CXXFLAGS}")
	link_libraries("${Yuni_LIBS}")

	message(STATUS "Sample: Multi-Threading / Queue Service basic usage")
	add_executable(jobs_00_basic_usage main.cpp)
endif(Yuni_FOUND)

