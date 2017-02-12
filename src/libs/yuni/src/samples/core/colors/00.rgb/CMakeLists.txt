

Find_Package(Yuni COMPONENTS core)
IF(Yuni_FOUND)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Yuni_CXXFLAGS}")
	link_libraries("${Yuni_LIBS}")

	Message(STATUS "Sample: Core / Colors / Conversion between several RGB color models")
	ADD_EXECUTABLE(color_00_rgb main.cpp)
ENDIF(Yuni_FOUND)
