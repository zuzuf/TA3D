if(NOT YUNI_AUTO_COMPILE_YUNI_CONFIG)
	add_subdirectory(yuni-config)
endif(NOT YUNI_AUTO_COMPILE_YUNI_CONFIG)

if(YUNI_MODULE_DOCS)
	add_subdirectory(yunidoc-make)
endif()

