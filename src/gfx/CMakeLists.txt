ADD_SUBDIRECTORY(gui)
ADD_SUBDIRECTORY(particles)

SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/libs/${CMAKE_SYSTEM_NAME}")

SET(SOURCES gfx.h gfx.cpp
            gfxfont.cpp gfxfont.h
            glfunc.h glfunc.cpp
            fx.base.h fx.base.cpp
            fx.particle.h fx.particle.cpp
            fx.manager.h fx.manager.cpp
            fx.h 
            gfxtexture.cpp gfxtexture.h)

ADD_LIBRARY(gfx STATIC ${SOURCES})
