

Find_Package(Yuni COMPONENTS core)
IF(Yuni_FOUND)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Yuni_CXXFLAGS}")
	link_libraries("${Yuni_LIBS}")

	Message(STATUS "Sample: Core / Logs / Writing to a file")
	ADD_EXECUTABLE(logs_01_file main.cpp)
ENDIF(Yuni_FOUND)
