SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/libs/${CMAKE_SYSTEM_NAME}")

SET(SOURCES particles.h particle.h 
            particlessystem.h particlessystem.cpp
            particlesengine.h particlesengine.cpp)

ADD_LIBRARY(particles STATIC ${SOURCES})
ADD_DEPENDENCIES(particles stdafx logs 3dtoolbox misc threads)
