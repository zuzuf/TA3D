

Find_Package(Yuni COMPONENTS core)
IF(Yuni_FOUND)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Yuni_CXXFLAGS}")
	link_libraries("${Yuni_LIBS}")

	Message(STATUS "Sample: Core / IO / Threaded iteration through a folder")
	ADD_EXECUTABLE(io_01_threadediterator main.cpp)
ENDIF(Yuni_FOUND)

