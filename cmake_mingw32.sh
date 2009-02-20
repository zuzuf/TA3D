cmake -G "MSYS Makefiles" \
      -DSDLIMAGE_LIBRARY:string=/lib/SDL_image.dll \
      -DSDL_INCLUDE_DIR:string=/include/SDL \
	  -DSDLIMAGE_INCLUDE_DIR:string=/include/ \
	  -DFREETYPE_LIBRARY:string=/lib/freetype6.dll \
      -DFREETYPE_INCLUDE_DIRS:string=/include/freetype2 \
	  -DFTGL_LIB:string=ftgl \
	  ./