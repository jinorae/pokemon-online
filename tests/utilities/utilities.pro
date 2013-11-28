#-------------------------------------------------
#
# Project created by QtCreator 2013-11-27T13:51:13
#
#-------------------------------------------------

CONFIG   += console
CONFIG   -= app_bundle

DESTDIR = $$PWD/../../bin
TEMPLATE = app


SOURCES += main.cpp \
    testinsensitivemap.cpp \
    testfunctions.cpp \
    testrankingtree.cpp \
    ../common/test.cpp \
    ../common/testrunner.cpp

INCLUDEPATH += ../../src/
INCLUDEPATH += ../common/

contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  DEFINES += QT5
  QT += widgets
  CONFIG += c++11
} else {
  QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"
}

include(../../src/Shared/Common.pri)

LIBS += $$utilities

TARGET = test-utilities

unix:!mac {
    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN'"
}

HEADERS += \
    testinsensitivemap.h \
    testfunctions.h \
    testrankingtree.h \
    ../common/test.h \
    ../common/testrunner.h

