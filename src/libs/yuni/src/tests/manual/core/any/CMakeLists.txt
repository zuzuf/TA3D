Message(STATUS "Test: Core/Any")
include_directories(${YUNI_INCLUDE})
ADD_EXECUTABLE(test_core_any_00 test_00.cpp)
target_link_libraries(test_core_any_00 ${YUNI_STATIC_CORE})
