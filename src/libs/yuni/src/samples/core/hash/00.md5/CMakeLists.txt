

find_package(Yuni COMPONENTS core)
if(Yuni_FOUND)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Yuni_CXXFLAGS}")
	link_libraries("${Yuni_LIBS}")

	message(STATUS "Sample: Core / Hash / Compute a MD5 checksum from a string")
	add_executable(hash_00_md5 main.cpp)
endif(Yuni_FOUND)

