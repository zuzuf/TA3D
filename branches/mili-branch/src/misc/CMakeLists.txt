SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/libs/${CMAKE_SYSTEM_NAME}")

SET(SOURCES settings.h settings.cpp
            interface.h interface.cpp
            lzw.h lzw.cpp
            vector.cpp vector.h
            hash_table.h hashtable.hxx
            superqueue.h superqueue.cpp
            paths.h paths.cpp)

ADD_LIBRARY(misc STATIC ${SOURCES})
