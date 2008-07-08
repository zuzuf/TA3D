SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/libs/${CMAKE_SYSTEM_NAME}")

SET(SOURCES ai.h ai.cpp
            pathfinding.h pathfinding.cpp)

ADD_LIBRARY(ai STATIC ${SOURCES})
ADD_DEPENDENCIES(ai stdafx misc threads)
