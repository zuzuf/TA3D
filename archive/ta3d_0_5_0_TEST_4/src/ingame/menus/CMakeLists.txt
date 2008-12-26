SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/libs/${CMAKE_SYSTEM_NAME}")

SET(SOURCES base.h base.cpp
            intro.h intro.cpp
            loading.h loading.cpp
            solo.h solo.cpp
            mapselector.h mapselector.cpp
            mainmenu.h mainmenu.cpp)

ADD_LIBRARY(menus STATIC ${SOURCES})
ADD_DEPENDENCIES(menus stdafx i18n misc)
