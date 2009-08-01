# --------------------------------------------------------------------
# --- ftgl
# --------------------------------------------------------------------
SET(ftgl_BIN ftgl)


SET(ftgl_SOURCES
		FTBuffer.cpp
		FTCharmap.cpp
		FTContour.cpp
		FTFace.cpp
		FTFont/FTExtrudeFont.cpp
		FTFont/FTPolygonFont.cpp
		FTFont/FTBufferFont.cpp
		FTFont/FTPixmapFont.cpp
		FTFont/FTBitmapFont.cpp
		FTFont/FTOutlineFont.cpp
		FTFont/FTFontGlue.cpp
		FTFont/FTFont.cpp
		FTFont/FTTextureFont.cpp
		FTGlyph/FTPolygonGlyph.cpp
		FTGlyph/FTTextureGlyph.cpp
		FTGlyph/FTGlyphGlue.cpp
		FTGlyph/FTPixmapGlyph.cpp
		FTGlyph/FTExtrudeGlyph.cpp
		FTGlyph/FTGlyph.cpp
		FTGlyph/FTBitmapGlyph.cpp
		FTGlyph/FTBufferGlyph.cpp
		FTGlyph/FTOutlineGlyph.cpp
		FTGlyphContainer.cpp
		FTLayout/FTSimpleLayout.cpp
		FTLayout/FTLayoutGlue.cpp
		FTLayout/FTLayout.cpp
		FTLibrary.cpp
		FTPoint.cpp
		FTSize.cpp
		FTVectoriser.cpp
		../config.h
		FTCharmap.h
		FTCharToGlyphIndexMap.h
		FTContour.h
		FTFace.h
		FTFont/FTTextureFontImpl.h
		FTFont/FTPolygonFontImpl.h
		FTFont/FTOutlineFontImpl.h
		FTFont/FTPixmapFontImpl.h
		FTFont/FTBitmapFontImpl.h
		FTFont/FTExtrudeFontImpl.h
		FTFont/FTFontImpl.h
		FTFont/FTBufferFontImpl.h
		FTGL/FTBitmapGlyph.h
		FTGL/FTBufferFont.h
		FTGL/ftgl.h
		FTGL/FTGlyph.h
		FTGL/FTLayout.h
		FTGL/FTGLPixmapFont.h
		FTGL/FTPolyGlyph.h
		FTGL/FTGLTextureFont.h
		FTGL/FTGLOutlineFont.h
		FTGL/FTOutlineGlyph.h
		FTGL/FTExtrdGlyph.h
		FTGL/FTPoint.h
		FTGL/FTSimpleLayout.h
		FTGL/FTBBox.h
		FTGL/FTPixmapGlyph.h
		FTGL/FTGLExtrdFont.h
		FTGL/FTGLBitmapFont.h
		FTGL/FTGLPolygonFont.h
		FTGL/FTBuffer.h
		FTGL/FTTextureGlyph.h
		FTGL/FTFont.h
		FTGL/FTBufferGlyph.h
		FTGlyph/FTTextureGlyphImpl.h
		FTGlyph/FTPolygonGlyphImpl.h
		FTGlyph/FTGlyphImpl.h
		FTGlyph/FTExtrudeGlyphImpl.h
		FTGlyph/FTPixmapGlyphImpl.h
		FTGlyph/FTBitmapGlyphImpl.h
		FTGlyph/FTBufferGlyphImpl.h
		FTGlyph/FTOutlineGlyphImpl.h
		FTGlyphContainer.h
		FTInternals.h
		FTLayout/FTSimpleLayoutImpl.h
		FTLayout/FTLayoutImpl.h
		FTLibrary.h
		FTList.h
		FTSize.h
		FTUnicode.h
		FTVector.h
		FTVectoriser.h)


add_library(${ftgl_BIN} STATIC ${ftgl_SOURCES})
