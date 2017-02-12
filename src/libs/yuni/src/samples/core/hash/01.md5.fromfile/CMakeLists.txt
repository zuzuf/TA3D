
find_package(Yuni COMPONENTS core)
if(Yuni_FOUND)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Yuni_CXXFLAGS}")
	link_libraries("${Yuni_LIBS}")

	message(STATUS "Sample: Core / Hash / Compute a MD5 checksum from a file")
	add_executable(hash_01_md5_fromfile main.cpp)
endif(Yuni_FOUND)

