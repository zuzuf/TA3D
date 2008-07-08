SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(SOURCES network.cpp network.h
            udpsock.cpp udpsock.h
            ta3dsock.cpp ta3dsock.h
            networkutils.h networkutils.cpp
            socketlist.h socketlist.cpp
            broadcastsock.cpp broadcastsock.h
            TA3D_Network.cpp TA3D_Network.h
            SocketClass.h SocketClass.cpp)
ADD_LIBRARY(network STATIC ${SOURCES})

ADD_DEPENDENCIES(network stdafx i18n 3dtoolbox misc hpi threads ingame)

