

find_package(Yuni COMPONENTS core)
if(Yuni_FOUND)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Yuni_CXXFLAGS}")
	link_libraries("${Yuni_LIBS}")

	message(STATUS "Sample: Core / Mathematics / Randomness / Typical use of a random number generator")
	add_executable(math_rn_00_simple main.cpp)
endif(Yuni_FOUND)

