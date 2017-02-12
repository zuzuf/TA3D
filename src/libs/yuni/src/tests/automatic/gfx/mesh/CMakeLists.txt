
include_directories(${YUNI_INCLUDE})
ADD_EXECUTABLE(test-gfx-mesh main.cpp)
target_link_libraries(test-gfx-mesh yuni-static-core yuni-static-gfx3d)
