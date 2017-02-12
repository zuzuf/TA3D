

Find_Package(Yuni COMPONENTS core)
IF(Yuni_FOUND)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Yuni_CXXFLAGS}")
	link_libraries("${Yuni_LIBS}")

	Message(STATUS "Sample: Multi-Threading / Using Condition Variable for Thread Synchronization")
	ADD_EXECUTABLE(threads_01_condition main.cpp)
ENDIF(Yuni_FOUND)

