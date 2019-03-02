TARGET = test-small-widgets
QT = core gui widgets

VALGRIND = true

FORMS += ../../forms/filepickerdialog.ui		\
	../../forms/filepickerwidget.ui

RESOURCES += ../../resources/resources.qrc

HEADERS  +=						\
	../../lib/core/TSettings.h			\
	../../src/customfilesystemmodel.h		\
	../../src/widgets/busywidget.h			\
	../../src/widgets/confirmationdialog.h		\
	../../src/widgets/elidedannotatedlabel.h	\
	../../src/widgets/elidedlabel.h			\
	../../src/widgets/filepickerdialog.h		\
	../../src/widgets/filepickerwidget.h		\
	../../src/widgets/helpwidget.h			\
	../../src/widgets/popuppushbutton.h		\
	../../tests/qtest-platform.h

SOURCES += test-small-widgets.cpp			\
	../../lib/core/TSettings.cpp			\
	../../src/customfilesystemmodel.cpp		\
	../../src/widgets/busywidget.cpp		\
	../../src/widgets/confirmationdialog.cpp	\
	../../src/widgets/elidedannotatedlabel.cpp	\
	../../src/widgets/elidedlabel.cpp		\
	../../src/widgets/filepickerdialog.cpp		\
	../../src/widgets/filepickerwidget.cpp		\
	../../src/widgets/helpwidget.cpp		\
	../../src/widgets/popuppushbutton.cpp

include(../tests-include.pro)
