Message(STATUS "Test: Core/String")
include_directories(${YUNI_INCLUDE})
ADD_EXECUTABLE(test_toolbox_string_00 string.cpp)
target_link_libraries(test_toolbox_string_00 ${YUNI_STATIC_CORE})
