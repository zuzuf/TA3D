ADD_SUBDIRECTORY(menus)
ADD_SUBDIRECTORY(weapons)

SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/libs/${CMAKE_SYSTEM_NAME}")

SET(SOURCES sidedata.h sidedata.cpp
            gamedata.h gamedata.cpp
            players.h players.cpp
            battle.h battle.init.cpp battle.preflight.cpp battle.cpp)

ADD_LIBRARY(ingame STATIC ${SOURCES})
