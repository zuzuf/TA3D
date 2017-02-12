
YMESSAGE(":: [Module] Network")

LIBYUNI_CONFIG_LIB("both" "net"        "yuni-static-net")



Set(SRC_NET
		net/net.h
		net/errors.h
		net/port.h
		net/port.hxx
		net/hostaddressport.h
		net/hostaddressport.hxx
		net/protocol.h
		net/protocol.cpp
		)
source_group("Network\\Protocol" FILES ${SRC_NET})

Add_Library(yuni-static-net STATIC ${SRC_NET})

# Setting output path
SET_TARGET_PROPERTIES(yuni-static-net PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${YUNI_OUTPUT_DIRECTORY}/lib")

# Installation
INSTALL(TARGETS yuni-static-net ARCHIVE DESTINATION lib/${YUNI_VERSIONED_INST_PATH})

# Install net-related headers
INSTALL(
	DIRECTORY net
	DESTINATION include/${YUNI_VERSIONED_INST_PATH}/yuni
	FILES_MATCHING
		PATTERN "*.h"
		PATTERN "*.hxx"
	PATTERN ".svn" EXCLUDE
	PATTERN "CMakeFiles" EXCLUDE
	PATTERN "cmake" EXCLUDE
)
