TEMPLATE = app
TARGET = myzip
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    core.cpp \
    tool.cpp \
    getopt.c
DEFINES -= UNICODE
#静态编译
QMAKE_LFLAGS  = -static

#http://stackoverflow.com/questions/3348711/add-a-define-to-qmake-with-a-value
#添加额外的宏定义 gcc -D_FILE_OFFSET_BITS=64
DEFINES += "_FILE_OFFSET_BITS=64"

include(deployment.pri)
qtcAddDeployment()


INCLUDEPATH += $$PWD/../libzip/src \
                $$PWD/../libutils/include \
                $$PWD/../zlib

HEADERS += \
    tool.h \
    getopt.h


win32{
    debug {
        DEFINES += _DEBUG
        DESTDIR = $$PWD/../build/debug
        TARGET = myzip
        LIBS += -L$$PWD/../lib/debug -llibzip_d
        LIBS += -L$$PWD/../lib/debug -lzlib_d
        LIBS += -L$$PWD/../lib/debug -llibutils_d
    }
    else:release {
        TARGET = myzip
        DESTDIR = $$PWD/../lib/release
        LIBS += -L$$PWD/../lib/debug -llibzip
        LIBS += -L$$PWD/../lib/release -lzlib
        LIBS += -L$$PWD/../lib/debug -llibutils
    }
}
else:unix{
    debug {
        DEFINES += _DEBUG
        DESTDIR = $$PWD/../build/debug
        TARGET = myzip
        LIBS += -L$$PWD/../lib/debug -llibzip_d
        LIBS += -L$$PWD/../lib/debug -lzlib_d
        LIBS += -L$$PWD/../lib/debug -llibutils_d
    }
    else:release {
        TARGET = myzip
        DESTDIR = $$PWD/../lib/release
        LIBS += -L$$PWD/../lib/debug -llibzip
        LIBS += -L$$PWD/../lib/release -lzlib
        LIBS += -L$$PWD/../lib/debug -llibutils
    }
}

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build/libzip/release/ -llibzip
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build/libzip/debug/ -llibzip
#else:unix: LIBS += -L$$PWD/../build/libzip/ -llibzip

#INCLUDEPATH += $$PWD/../build/libzip
#DEPENDPATH += $$PWD/../build/libzip

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
