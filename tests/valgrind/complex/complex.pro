QT = core

TEMPLATE = app
TARGET = potential-memleaks

HEADERS = app.h

SOURCES = complex.cpp

# Set up the build directory
include(../valgrind-include.pro)
