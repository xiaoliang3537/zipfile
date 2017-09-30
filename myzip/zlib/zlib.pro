#zlib 1.2.8 project
QT = ""
#TARGET = zlib
TEMPLATE = lib
CONFIG += staticlib

win32{
    debug {
        DEFINES += _DEBUG
        TARGET = zlib_d
        DESTDIR = $$PWD/../lib/debug
    }
    else:release {
        TARGET = zlib
        DESTDIR = $$PWD/../lib/release
    }
}
else:unix{
    debug {
        TARGET = zlib_d
        DESTDIR = $$PWD/../lib/debug
    }
    else:release {
        TARGET = zlib
        DESTDIR = $$PWD/../lib/release
    }
}

SOURCES += \
    adler32.c \
    compress.c\
    crc32.c   \
    deflate.c \
    gzclose.c \
    gzlib.c   \
    gzread.c  \
    gzwrite.c \
    inflate.c \
    infback.c \
    inftrees.c\
    inffast.c \
    trees.c   \
    uncompr.c \
    zutil.c \
    unzip.c \
    ioapi.c

HEADERS  += \
    zconf.h   \
    zlib.h    \
    crc32.h   \
    deflate.h \
    gzguts.h  \
    inffast.h \
    inffixed.h\
    inflate.h \
    inftrees.h\
    trees.h   \
    zutil.h \
    unzip.h \
    ioapi.h

