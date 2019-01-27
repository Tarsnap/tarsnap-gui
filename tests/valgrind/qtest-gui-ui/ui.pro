QT = core testlib
QT += gui widgets

TEMPLATE = app
TARGET = potential-memleaks

FORMS +=	../../../forms/filepickerwidget.ui

SOURCES +=	ui.cpp

# Set up the build directory
include(../valgrind-include.pro)
