CONFIG += c++11
TOPDIR = ../../../

# This path is relative to this file, not the current working directory.
include(../../build-flags.pri)


### Shared build-dir
UI_DIR      = $$TOPDIR/build/valgrind/
MOC_DIR     = $$TOPDIR/build/valgrind/
RCC_DIR     = $$TOPDIR/build/valgrind/
OBJECTS_DIR = $$TOPDIR/build/valgrind/
