Message(STATUS "Test: Core/Allocator")
include_directories(${YUNI_INCLUDE})
ADD_EXECUTABLE(test_core_allocator_00 test_00.cpp)
target_link_libraries(test_core_allocator_00 ${YUNI_STATIC_CORE})
