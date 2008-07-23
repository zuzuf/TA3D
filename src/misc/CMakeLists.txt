SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/libs/${CMAKE_SYSTEM_NAME}")

SET(SOURCES string.h string.cpp
            paths.h paths.cpp
            files.h files.cpp
            resources.h resources.cpp
            settings.h settings.cpp
            interface.h interface.cpp
            lzw.h lzw.cpp
            hash_table.h hashtable.hxx
            superqueue.h superqueue.cpp
            osinfo.h osinfo.cpp
            application.h application.cpp)

ADD_LIBRARY(misc STATIC ${SOURCES})

SET(SOURCES vector.cpp vector.h
            matrix.h matrix.cpp
            material.light.h material.light.cpp
            camera.h camera.cpp)

ADD_LIBRARY(3dtoolbox STATIC ${SOURCES})


ADD_LIBRARY(tdf STATIC tdf.h tdf.cpp)

ADD_LIBRARY(ta3dmath STATIC math.h math.cpp)
