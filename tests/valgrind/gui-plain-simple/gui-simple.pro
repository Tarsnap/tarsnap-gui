QT = core gui widgets

TEMPLATE = app
TARGET = potential-memleaks

SOURCES = gui-simple.cpp

# Set up the build directory
include(../valgrind-include.pro)
