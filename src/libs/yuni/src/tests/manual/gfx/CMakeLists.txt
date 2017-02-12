
IF(YUNI_MODULE_GFX3D)
	PROJECT(manual-gfx-marchingcubes)
	include_directories(${YUNI_INCLUDE})

	ADD_EXECUTABLE(manual-gfx-marchingcubes-yunilogo MACOSX_BUNDLE yunilogo.cpp)
# 	ADD_EXECUTABLE(manual-gfx-marchingcubes-ballsmerging MACOSX_BUNDLE ballsmerging.cpp)
	ADD_EXECUTABLE(manual-gfx-marchingcubes-box MACOSX_BUNDLE box.cpp)
	ADD_EXECUTABLE(manual-gfx-scaleimage MACOSX_BUNDLE scaleimage.cpp)

	SET(MACOSX_BUNDLE_BUNDLE_NAME "Yuni - Test - Marching Cubes")
	SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "yunihw")
	SET(MACOSX_BUNDLE_ICON_FILE "sample.icns")

	target_link_libraries(manual-gfx-marchingcubes-yunilogo ${YUNI_STATIC_CORE} ${YUNI_STATIC_GFX_CORE} ${YUNI_STATIC_GFX3D})
# 	target_link_libraries(manual-gfx-marchingcubes-ballsmerging  ${YUNI_STATIC_CORE} ${YUNI_STATIC_GFX_CORE} ${YUNI_STATIC_GFX3D})
 	target_link_libraries(manual-gfx-marchingcubes-box  ${YUNI_STATIC_CORE} ${YUNI_STATIC_GFX_CORE} ${YUNI_STATIC_GFX3D})
	target_link_libraries(manual-gfx-scaleimage  ${YUNI_STATIC_CORE} ${YUNI_STATIC_GFX_CORE} ${YUNI_STATIC_GFX3D})

ENDIF(YUNI_MODULE_GFX3D)
