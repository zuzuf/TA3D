SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/libs/${CMAKE_SYSTEM_NAME}")

SET(SOURCES skin.h skin.cpp
            skin.object.h skin.object.cpp)

ADD_LIBRARY(gui STATIC ${SOURCES})
ADD_DEPENDENCIES(gui stdafx logs misc threads)
