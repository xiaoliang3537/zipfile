#TEMPLATE = app
#CONFIG += console

TEMPLATE = lib
CONFIG   += staticlib

CONFIG -= app_bundle
CONFIG -= qt
DEFINES -= UNICODE

SOURCES += \
    src/3Des.cpp \
    src/Crypto.cpp \
    src/Md5.cpp

#http://stackoverflow.com/questions/3348711/add-a-define-to-qmake-with-a-value
#添加额外的宏定义 gcc -D_FILE_OFFSET_BITS=64
DEFINES += "_FILE_OFFSET_BITS=64"

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    include/3Des.h \
    include/CryptoDes.h \
    include/Debug.h \
    include/DebugMem.h \
    include/FileUtils.h \
    include/Md5.h \
    include/PathUtils.h \
    include/StringUtils.h \
    include/Utils.h

win32{
    debug {
        DEFINES += _DEBUG
        TARGET = libutils_d
        DESTDIR = $$PWD/../lib/debug
    }
    else:release {
        TARGET = libutils
        DESTDIR = $$PWD/../lib/release
    }
}
else:unix{
    debug {
        DEFINES += _DEBUG
        TARGET = libutils_d
        DESTDIR = $$PWD/../lib/debug
    }
    else:release {
        TARGET = libutils
        DESTDIR = $$PWD/../lib/release
    }
}

#win32:LIBS += Kernel32.lib

