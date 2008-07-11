SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/libs/${CMAKE_SYSTEM_NAME}")

SET(SOURCES base.h base.cpp
            object.h object.cpp
            skin.h skin.cpp
            skin.object.h skin.object.cpp
            wnd.h wnd.cpp
            area.h area.cpp)

ADD_LIBRARY(gui STATIC ${SOURCES})
ADD_DEPENDENCIES(gui stdafx logs misc threads)
