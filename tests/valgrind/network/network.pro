QT = core
QT += network

TEMPLATE = app
TARGET = potential-memleaks

SOURCES = network.cpp

# Set up the build directory
include(../valgrind-include.pro)
