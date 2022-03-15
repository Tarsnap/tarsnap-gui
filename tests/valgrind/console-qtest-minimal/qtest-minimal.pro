QT = core testlib

TEMPLATE = app
TARGET = potential-memleaks

SOURCES = qtest-minimal.cpp

# Set up the build directory
include(../valgrind-include.pro)
