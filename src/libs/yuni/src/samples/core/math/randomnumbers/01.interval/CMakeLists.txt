

find_package(Yuni COMPONENTS core)
if(Yuni_FOUND)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Yuni_CXXFLAGS}")
	link_libraries("${Yuni_LIBS}")

	message(STATUS "Sample: Core / Mathematics / Randomness / Random numbers within a range")
	add_executable(math_rn_01_range main.cpp)
endif(Yuni_FOUND)

