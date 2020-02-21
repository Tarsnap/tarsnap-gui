QT = core
QT += gui widgets network

TEMPLATE = app
TARGET = potential-memleaks

SOURCES = gui-network.cpp

# Set up the build directory
include(../valgrind-include.pro)
