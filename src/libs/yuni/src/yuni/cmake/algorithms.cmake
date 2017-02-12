
YMESSAGE(":: [Module] Algorithms")

LIBYUNI_CONFIG_LIB("both" "algorithms" "yuni-static-algorithms")


# Devices
set(SRC_ALGORITHMS
		algorithm/luhn.h algorithm/luhn.cpp algorithm/luhn.hxx
		)
source_group(Algorithms FILES ${SRC_ALGORITHMS})



add_library(yuni-static-algorithms STATIC
			yuni.h
			${SRC_ALGORITHMS}
)

# Setting output path
SET_TARGET_PROPERTIES(yuni-static-algorithms PROPERTIES 
		ARCHIVE_OUTPUT_DIRECTORY "${YUNI_OUTPUT_DIRECTORY}/lib")

# Installation
INSTALL(TARGETS yuni-static-algorithms ARCHIVE DESTINATION lib/${YUNI_VERSIONED_INST_PATH})

# Install Algo-related headers
INSTALL(
	DIRECTORY algorithm
	DESTINATION include/${YUNI_VERSIONED_INST_PATH}
	FILES_MATCHING
		PATTERN "*.h"
		PATTERN "*.hxx"
	PATTERN ".svn" EXCLUDE
	PATTERN "CMakeFiles" EXCLUDE
	PATTERN "cmake" EXCLUDE
)

# Linking deps
target_link_libraries(yuni-static-algorithms yuni-static-core)


