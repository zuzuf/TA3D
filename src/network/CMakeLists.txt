SET(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/libs/${CMAKE_SYSTEM_NAME}")
SET(SOURCES network.cpp network.h
            socket.cpp socket.h
            socket.tcp.cpp socket.tcp.h
            socket.broadcast.cpp socket.broadcast.h
            socket.udp.cpp socket.udp.h
            ta3dsock.cpp ta3dsock.h
            networkutils.h networkutils.cpp
            socketlist.h socketlist.cpp
            TA3D_Network.cpp TA3D_Network.h
            netclient.cpp netclient.h)
ADD_LIBRARY(network STATIC ${SOURCES})
