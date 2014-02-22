QT = core gui
macx:CONFIG -= app_bundle
win32:CONFIG += console
TEMPLATE = app
TARGET = cplusplus-frontend
DESTDIR = ./

include(../../../qtcreator.pri)
include($$IDE_SOURCE_TREE/src/libs/cplusplus/cplusplus-lib.pri)
include($$IDE_SOURCE_TREE/tests/auto/qttestrpath.pri)
include(../../../src/tools/cplusplus-tools-utils/cplusplus-tools-utils.pri)

SOURCES += cplusplus-frontend.cpp
