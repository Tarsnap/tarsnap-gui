TARGET = test-small-widgets
QT = core gui widgets

VALGRIND = true

FORMS += ../../forms/filepickerdialog.ui		\
	../../forms/filepickerwidget.ui

HEADERS  +=						\
	../../lib/core/TSettings.h			\
	../../src/customfilesystemmodel.h		\
	../../src/widgets/elidedlabel.h			\
	../../src/widgets/filepickerdialog.h		\
	../../src/widgets/filepickerwidget.h		\
	../../tests/qtest-platform.h

SOURCES += test-small-widgets.cpp			\
	../../lib/core/TSettings.cpp			\
	../../src/customfilesystemmodel.cpp		\
	../../src/widgets/elidedlabel.cpp		\
	../../src/widgets/filepickerdialog.cpp		\
	../../src/widgets/filepickerwidget.cpp

include(../tests-include.pro)
