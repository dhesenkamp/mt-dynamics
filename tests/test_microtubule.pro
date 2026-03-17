QT += testlib
QT -= gui

CONFIG += c++17 console testcase
CONFIG -= app_bundle

# Point back up into the main project's include folder to access the files to test
INCLUDEPATH += ../include

SOURCES += \
    test_microtubule.cpp \
    ../src/microtubule.cpp

HEADERS += \
    ../include/microtubule.h