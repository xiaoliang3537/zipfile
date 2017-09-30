#-------------------------------------------------
#
# Project created by QtCreator 2014-09-24T16:28:19
#
#-------------------------------------------------

QT       -= core gui

TARGET = libzip
TEMPLATE = lib
CONFIG += staticlib
#CONFIG += -static
#QMAKE_CXXFLAGS += -static -static-libgcc -static-libstdc++  -pthread

#TEMPLATE = app
#CONFIG += console
#CONFIG -= qt

SOURCES += \
   test_zipfile.cpp \
    centraldir.cpp \
    zipfile.cpp \
    zip.c \
    ioapi.c

HEADERS += zipfile.h \
    private.h \
    zip.h \
    ioapi.h \
    crypt.h


#unix:!macx:!symbian: LIBS += -L$$PWD/../zlib-1.2.8/ -lz
unix:!macx:!symbian: LIBS += -L$$PWD/../zlib/src -lz

INCLUDEPATH += $$PWD/../zlib/src
DEPENDPATH += $$PWD/../zlib/src

unix:!macx:!symbian: PRE_TARGETDEPS += $$PWD/../zlib/src/libz.a
