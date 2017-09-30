#TEMPLATE = app
#CONFIG += console
TEMPLATE = lib
CONFIG   += staticlib
CONFIG -= app_bundle
CONFIG -= qt
DEFINES -= UNICODE

INCLUDEPATH += $$PWD/../zlib
DEPENDPATH += $$PWD/../zlib

SOURCES += \
    src/centraldir.cpp \
    #src/test_zipfile.cpp \
    src/zipfile.cpp \
    src/ioapi.c \
    src/zip.c \
    src/fnmatch.c

#http://stackoverflow.com/questions/3348711/add-a-define-to-qmake-with-a-value
#添加额外的宏定义 gcc -D_FILE_OFFSET_BITS=64
DEFINES += "_FILE_OFFSET_BITS=64"

include(deployment.pri)
qtcAddDeployment()

SUBDIRS += \
    src/libzip.pro

HEADERS += \
    src/crypt.h \
    src/ioapi.h \
    src/private.h \
    src/zip.h \
    src/zipfile.h \
    src/fnmatch.h

win32{
    debug {
        DEFINES += _DEBUG
        TARGET = libzip_d
        DESTDIR = $$PWD/../lib/debug
        LIBS += -L$$PWD\..\build\zlib\debug -lzlib
    }
    else:release {
        TARGET = libzip
        DESTDIR = $$PWD/../lib/release
        LIBS += -L$$PWD\..\build\zlib\release -lzlib
    }
}
else:unix{
    debug {
        DEFINES += _DEBUG
        TARGET = libzip_d
        DESTDIR = $$PWD/../lib/debug
        LIBS += -L$$PWD\..\build\zlib\debug -lzlib
    }
    else:release {
        TARGET = libzip
        DESTDIR = $$PWD/../lib/release
        LIBS += -L$$PWD\..\build\zlib\release -lzlib
    }
}

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build/zlib/release/ -lzlib
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build/zlib/debug/ -lzlib
#else:unix: LIBS += -L$$PWD/../build/zlib/ -lzlib



#win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build/zlib/release/libzlib.a
#else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build/zlib/debug/libzlib.a
#else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build/zlib/release/zlib.lib
#else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build/zlib/debug/zlib.lib
#else:unix: PRE_TARGETDEPS += $$PWD/../build/zlib/libzlib.a
