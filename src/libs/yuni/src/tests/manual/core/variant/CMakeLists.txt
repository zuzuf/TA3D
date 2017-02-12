Message(STATUS "Test: Core/Variant")
include_directories(${YUNI_INCLUDE})
ADD_EXECUTABLE(test_core_variant_00 test_00.cpp)
target_link_libraries(test_core_variant_00 ${YUNI_STATIC_CORE})
