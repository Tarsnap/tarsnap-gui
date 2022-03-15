QT = core sql

TEMPLATE = app
TARGET = potential-memleaks

SOURCES = simple.cpp

# Set up the build directory
include(../valgrind-include.pro)
