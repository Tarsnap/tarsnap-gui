QT = core testlib
QT += gui widgets

TEMPLATE = app
TARGET = potential-memleaks

SOURCES = qtest-gui-simple.cpp

# Set up the build directory
include(../valgrind-include.pro)
