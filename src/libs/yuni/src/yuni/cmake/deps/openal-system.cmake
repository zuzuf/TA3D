
YMESSAGE("      Using OpenAL from system")

# Just check the al.h header and library in standard includes paths.
find_path(OPENAL_INCLUDE_DIR al.h 
	PATH_SUFFIXES include/AL include/OpenAL include
	PATHS
		/usr/local
		/usr
		[HKEY_LOCAL_MACHINE\\SOFTWARE\\Creative\ Labs\\OpenAL\ 1.1\ Software\ Development\ Kit\\1.00.0000;InstallDir]
)


# Mac OS X
if(APPLE)

	# Frameworks
	LIBYUNI_CONFIG_FRAMEWORK("both" "audio" OpenAL)

else()

	find_library(OPENAL_LIBRARY 
		NAMES OpenAL al openal OpenAL32
		PATH_SUFFIXES lib64 lib libs64 libs libs/Win32 libs/Win64
		PATHS
			/usr/local
			/usr
			[HKEY_LOCAL_MACHINE\\SOFTWARE\\Creative\ Labs\\OpenAL\ 1.1\ Software\ Development\ Kit\\1.00.0000;InstallDir]
	)

endif()

LIBYUNI_CONFIG_INCLUDE_PATH("both" "audio" "${OPENAL_INCLUDE_DIR}")
LIBYUNI_CONFIG_LIB("both" "audio" "${OPENAL_LIBRARY}")
include_directories("${OPENAL_INCLUDE_DIR}")

