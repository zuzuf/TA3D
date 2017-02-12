

find_package(Yuni COMPONENTS audio)
if(Yuni_FOUND)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Yuni_CXXFLAGS}")
	add_definitions(-D__STDC_CONSTANT_MACROS)
	link_libraries("${Yuni_LIBS}")

	message(STATUS "Sample: Audio / Play a sound on a moving emitter")
	add_executable(audio_01_movingemitter 01.movingemitter.cpp)
endif(Yuni_FOUND)

