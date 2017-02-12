
IF(YUNI_MODULE_GFX3D)
	include_directories(${YUNI_INCLUDE})
	ADD_EXECUTABLE(test-gfx-objects main.cpp)
	target_link_libraries(test-gfx-objects ${YUNI_STATIC_CORE}
	${YUNI_STATIC_GFX_CORE} ${YUNI_STATIC_GFX3D})
ENDIF(YUNI_MODULE_GFX3D)

