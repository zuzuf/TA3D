# -------------------------------------------------
# Project created by QtCreator 2009-03-30T19:47:07
# -------------------------------------------------
QT += opengl \
    webkit
TARGET = 3DMEditor2
LIBS += -lz
win32:LIBS += -lglew32.dll
unix:LIBS += -lGLEW -lGLU
macos:LIBS += -lGLEW
QMAKE_CXXFLAGS_DEBUG += --no-warnings
TEMPLATE = app
SOURCES += src/main.cpp \
    src/mainwindow.cpp \
    src/gfx.cpp \
    src/misc/vector.cpp \
    src/misc/matrix.cpp \
    src/misc/camera.cpp \
    src/misc/math.cpp \
    src/misc/grid.cpp \
    src/aboutwindow.cpp \
    src/misc/material.light.cpp \
    src/mesh.cpp \
    src/3ds.cpp \
    src/obj.cpp \
    src/geometrygraph.cpp \
    src/program.cpp \
    src/textureviewer.cpp \
    src/surfaceproperties.cpp \
    src/shadereditor.cpp \
    src/helpviewer.cpp \
    src/imagelistview.cpp \
    src/progressdialog.cpp \
    src/meshtree.cpp \
    src/springmodelloader.cpp \
    src/toolbox.cpp \
    src/flowlayout.cpp \
    src/animation.cpp \
    src/luaeditor.cpp \
    src/scripts/lua.thread.cpp \
    src/scripts/script.interface.cpp \
    src/scripts/unit.script.cpp \
    src/scripts/unit.script.func.cpp \
    src/scripts/unit.script.interface.cpp \
    src/lua/lzio.c \
    src/lua/lobject.c \
    src/lua/lundump.c \
    src/lua/print.c \
    src/lua/lmem.c \
    src/lua/lstate.c \
    src/lua/lapi.c \
    src/lua/lparser.c \
    src/lua/ldebug.c \
    src/lua/ltable.c \
    src/lua/linit.c \
    src/lua/lvm.c \
    src/lua/lstring.c \
    src/lua/ldump.c \
    src/lua/lmathlib.c \
    src/lua/ltablib.c \
    src/lua/lbaselib.c \
    src/lua/ldblib.c \
    src/lua/llex.c \
    src/lua/loslib.c \
    src/lua/liolib.c \
    src/lua/lfunc.c \
    src/lua/lcode.c \
    src/lua/lstrlib.c \
    src/lua/loadlib.c \
    src/lua/lauxlib.c \
    src/lua/ldo.c \
    src/lua/lopcodes.c \
    src/lua/ltm.c \
    src/lua/lgc.c \
    src/ambientocclusion.cpp \
    src/3do.cpp
HEADERS += src/mainwindow.h \
    src/config.h \
    src/gfx.h \
    src/misc/vector.h \
    src/misc/matrix.h \
    src/misc/camera.h \
    src/misc/math.h \
    src/misc/grid.h \
    src/misc/grid.hxx \
    src/types.h \
    src/aboutwindow.h \
    src/misc/material.light.h \
    src/mesh.h \
    src/3ds.h \
    src/obj.h \
    src/geometrygraph.h \
    src/program.h \
    src/textureviewer.h \
    src/surfaceproperties.h \
    src/shadereditor.h \
    src/helpviewer.h \
    src/imagelistview.h \
    src/progressdialog.h \
    src/meshtree.h \
    src/springmodelloader.h \
    src/toolbox.h \
    src/flowlayout.h \
    src/animation.h \
    src/luaeditor.h \
    src/scripts/lua.thread.h \
    src/scripts/script.interface.h \
    src/scripts/unit.script.h \
    src/scripts/unit.script.interface.h \
    src/logs.h \
    src/lua/llex.h \
    src/lua/ldo.h \
    src/lua/ldebug.h \
    src/lua/ltable.h \
    src/lua/lualib.h \
    src/lua/luaconf.h \
    src/lua/lobject.h \
    src/lua/lapi.h \
    src/lua/lopcodes.h \
    src/lua/lgc.h \
    src/lua/lcode.h \
    src/lua/lauxlib.h \
    src/lua/lfunc.h \
    src/lua/ltm.h \
    src/lua/lmem.h \
    src/lua/lparser.h \
    src/lua/lvm.h \
    src/lua/lundump.h \
    src/lua/lstring.h \
    src/lua/lzio.h \
    src/lua/lstate.h \
    src/lua/lua.h \
    src/lua/llimits.h \
    src/lua/lua.hpp
TRANSLATIONS = i18n/3dmeditor_fr.ts \
    i18n/3dmeditor_en.ts
