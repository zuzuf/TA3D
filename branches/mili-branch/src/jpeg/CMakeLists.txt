SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/libs/${CMAKE_SYSTEM_NAME}")

SET(SOURCES decode.cpp
            encode.cpp
            io.cpp
            ta3d_jpg.cpp ta3d_jpg.h
            dct.h internal.h)

ADD_LIBRARY(tajpeg STATIC ${SOURCES})
