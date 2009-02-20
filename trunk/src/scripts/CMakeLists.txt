SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/libs/${CMAKE_SYSTEM_NAME}")

SET(SOURCES cob.cpp cob.h
            lua.thread.h lua.thread.cpp
            lua.env.h lua.env.cpp
            draw.list.h draw.list.cpp
            script.h script.cpp)

ADD_LIBRARY(scripts STATIC ${SOURCES})
