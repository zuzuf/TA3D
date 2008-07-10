SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../bin/libs/${CMAKE_SYSTEM_NAME}")

SET(SOURCES weapons.h weapons.cpp
            weapons.def.h weapons.def.cpp
            weapons.ingame.h weapons.ingame.cpp
            weapons.manager.h weapons.manager.cpp
            weapons.single.h weapons.single.cpp)

ADD_LIBRARY(weapons STATIC ${SOURCES})
ADD_DEPENDENCIES(weapons stdafx i18n misc 3dtoolbox threads gfx gui misc ingame)
