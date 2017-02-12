
include_directories(${YUNI_INCLUDE})
ADD_EXECUTABLE(test-gfx-points-2d main.cpp)
target_link_libraries(test-gfx-points-2d ${YUNI_STATIC_CORE})

