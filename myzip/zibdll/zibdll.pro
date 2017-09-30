TEMPLATE = lib
#TARGET = zlib
#CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DEFINES -= UNICODE

#include(deployment.pri)
#qtcAddDeployment()

##http://stackoverflow.com/questions/3348711/add-a-define-to-qmake-with-a-value
##添加额外的宏定义 gcc -D_FILE_OFFSET_BITS=64
DEFINES += "_FILE_OFFSET_BITS=64"

INCLUDEPATH += $$PWD/../libzip/src
INCLUDEPATH += $$PWD/../libzip/zlib128-dll/include
INCLUDEPATH += $$PWD/../libutils/include
INCLUDEPATH += $$PWD/../zlib

DEPENDPATH += $$PWD/../build/libzip

SOURCES += \
     $$PWD/../libzip/src/centraldir.cpp \
    #src/test_zipfile.cpp \
    $$PWD/../libzip/src/zipfile.cpp \
    $$PWD/../libzip/src/ioapi.c \
    $$PWD/../libzip/src/zip.c \
    $$PWD/../libzip/src/fnmatch.c

HEADERS += \
    $$PWD/../libzip/src/crypt.h \
    $$PWD/../libzip/src/ioapi.h \
    $$PWD/../libzip/src/private.h \
    $$PWD/../libzip/src/zip.h \
    $$PWD/../libzip/src/zipfile.h \
    $$PWD/../libzip/src/fnmatch.h


#QMAKE_CFLAGS = '-fvisibility=hidden'
QMAKE_CXXFLAGS += -fvisibility=hidden

win32{
    debug {
        DEFINES += _DEBUG
        DESTDIR = $$PWD/../lib/debug
        TARGET = zlib
        LIBS += -L$$PWD/../lib/debug -llibzip_d
        LIBS += -L$$PWD/../lib/debug -lzlib_d
        LIBS += -L$$PWD/../lib/debug -llibutils_d
    }
    else:release {
        TARGET = zlib
        DESTDIR = $$PWD/../lib/release
        LIBS += -L$$PWD/../lib/debug -llibzip
        LIBS += -L$$PWD/../lib/release -lzlib
        LIBS += -L$$PWD/../lib/debug -llibutils
    }
}
else:unix{
    debug {
        DEFINES += _DEBUG
        DESTDIR = $$PWD/../lib/debug
        TARGET = zlib
        LIBS += -L$$PWD/../lib/debug -llibzip_d
        LIBS += -L$$PWD/../lib/debug -lzlib_d
        LIBS += -L$$PWD/../lib/debug -llibutils_d
    }
    else:release {
        TARGET = zlib
        DESTDIR = $$PWD/../lib/release
        LIBS += -L$$PWD/../lib/debug -llibzip
        LIBS += -L$$PWD/../lib/release -lzlib
        LIBS += -L$$PWD/../lib/debug -llibutils
    }
}

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build/libzip/release/ -llibzip
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build/libzip/debug/ -llibzip
#else:unix: LIBS += -L$$PWD/../build/libzip/ -llibzip


#win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build/libzip/release/liblibzip.a
#else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build/libzip/debug/liblibzip.a
#else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build/libzip/release/libzip.lib
#else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build/libzip/debug/libzip.lib
#else:unix: PRE_TARGETDEPS += $$PWD/../build/libzip/liblibzip.a

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build/libutils/release/ -llibutils
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build/libutils/debug/ -llibutils
#else:unix: LIBS += -L$$PWD/../build/libutils/ -llibutils

#INCLUDEPATH += $$PWD/../build/libutils
#DEPENDPATH += $$PWD/../build/libutils

#win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build/libutils/release/liblibutils.a
#else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build/libutils/debug/liblibutils.a
#else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build/libutils/release/libutils.lib
#else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build/libutils/debug/libutils.lib
#else:unix: PRE_TARGETDEPS += $$PWD/../build/libutils/liblibutils.a

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build/zlib/release/ -lzlib
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build/zlib/debug/ -lzlib
#else:unix: LIBS += -L$$PWD/../build/zlib/ -lzlib

#INCLUDEPATH += $$PWD/../zlib
#DEPENDPATH += $$PWD/../zlib

#win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build/zlib/release/libzlib.a
#else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build/zlib/debug/libzlib.a
#else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build/zlib/release/zlib.lib
#else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build/zlib/debug/zlib.lib
#else:unix: PRE_TARGETDEPS += $$PWD/../build/zlib/libzlib.a



