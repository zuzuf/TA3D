# -------------------------------------------------
# Project created by QtCreator 2009-03-30T19:47:07
# -------------------------------------------------
QT += opengl
CONFIG -= release
CONFIG += debug
TARGET = 3DMEditor2
TEMPLATE = app
SOURCES += src/main.cpp \
    src/mainwindow.cpp \
    src/qpopup.cpp \
    src/gfx.cpp \
    src/misc/vector.cpp \
    src/misc/matrix.cpp \
    src/misc/camera.cpp \
    src/misc/math.cpp \
    src/aboutwindow.cpp \
    src/misc/material.light.cpp
HEADERS += src/mainwindow.h \
    src/config.h \
    src/qpopup.h \
    src/gfx.h \
    src/misc/vector.h \
    src/misc/matrix.h \
    src/misc/camera.h \
    src/misc/math.h \
    src/types.h \
    src/aboutwindow.h \
    src/misc/material.light.h
TRANSLATIONS = i18n/3dmeditor_fr.ts \
    i18n/3dmeditor_en.ts
