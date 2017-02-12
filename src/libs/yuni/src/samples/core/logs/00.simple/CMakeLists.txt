

Find_Package(Yuni COMPONENTS core)
IF(Yuni_FOUND)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Yuni_CXXFLAGS}")
	link_libraries("${Yuni_LIBS}")

	Message(STATUS "Sample: Core / Logs / 'Hello World' with several verbosity levels")
	ADD_EXECUTABLE(logs_00_simple main.cpp)
ENDIF(Yuni_FOUND)
