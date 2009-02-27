SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/libs/${CMAKE_SYSTEM_NAME}")

SET(SOURCES cob.cpp cob.h
            cob.vm.h cob.vm.cpp
            lua.thread.h lua.thread.cpp
            lua.env.h lua.env.cpp
            lua.chunk.h lua.chunk.cpp
            draw.list.h draw.list.cpp
            unit.script.h unit.script.cpp
            unit.script.func.cpp
            script.interface.h script.interface.cpp
            unit.script.interface.h unit.script.interface.cpp
            ai.script.h ai.script.cpp
            script.data.h script.data.cpp
            unit.defines.h
            script.h script.cpp)

ADD_LIBRARY(scripts STATIC ${SOURCES})
