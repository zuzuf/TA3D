
YMESSAGE(":: [Module] Network Messages  (EXPERIMENTAL, FOR ADVANCED USERS ONLY)")

LIBYUNI_CONFIG_LIB("both" "net-messages"        "yuni-static-net-messages")



Set(SRC_NET
	net/message/connection.h
	net/message/queueservice.cpp
	net/message/queueservice.h
	net/message/queueservice.hxx
	net/message/worker.h
	net/message/worker.hxx
	net/message/worker.cpp
	net/message/transport.h
	net/message/transport
	net/message/transport/layer.h
	net/message/transport/transport.h
	net/message/transport/transport.hxx
	net/message/transport/tcp.h
	net/message/transport/tcp.cpp
	)
source_group("Network\\Messages" FILES ${SRC_NET})

Add_Library(yuni-static-net-messages STATIC ${SRC_NET})

# Setting output path
SET_TARGET_PROPERTIES(yuni-static-net-messages PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${YUNI_OUTPUT_DIRECTORY}/lib")

# Installation
INSTALL(TARGETS yuni-static-net-messages ARCHIVE DESTINATION lib/${YUNI_VERSIONED_INST_PATH})

# Install net-related headers
INSTALL(
	DIRECTORY net/server
	DESTINATION include/${YUNI_VERSIONED_INST_PATH}/yuni
	FILES_MATCHING
		PATTERN "*.h"
		PATTERN "*.hxx"
	PATTERN ".svn" EXCLUDE
	PATTERN "CMakeFiles" EXCLUDE
	PATTERN "cmake" EXCLUDE
)
