QT = core testlib
QT += gui widgets network

TEMPLATE = app
TARGET = potential-memleaks

SOURCES = qtest-gui-complex.cpp

# Set up the build directory
include(../valgrind-include.pro)
