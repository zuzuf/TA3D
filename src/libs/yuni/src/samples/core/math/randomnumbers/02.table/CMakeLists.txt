

find_package(Yuni COMPONENTS core)
if(Yuni_FOUND)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Yuni_CXXFLAGS}")
	link_libraries("${Yuni_LIBS}")

	message(STATUS "Sample: Core / Mathematics / Randomness / Random number table")
	add_executable(math_rn_02_table main.cpp)
endif(Yuni_FOUND)

