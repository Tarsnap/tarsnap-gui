QT = core

TEMPLATE = app
TARGET = potential-memleaks

SOURCES = minimal.cpp

# Set up the build directory
include(../valgrind-include.pro)
