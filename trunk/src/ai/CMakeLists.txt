SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/libs/${CMAKE_SYSTEM_NAME}")

SET(SOURCES ai.h ai.cpp
            brain.h brain.cpp
            weight.h weight.cpp
            ai.controller.h ai.controller.cpp
            pathfinding.h pathfinding.cpp)

ADD_LIBRARY(ai STATIC ${SOURCES})
