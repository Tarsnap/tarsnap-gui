TARGET = test-small-widgets
QT = core gui widgets
TOPDIR = ../..

VALGRIND = true

FORMS +=								\
	$$TOPDIR/forms/filepickerdialog.ui				\
	$$TOPDIR/forms/filepickerwidget.ui				\
	$$TOPDIR/forms/stoptasksdialog.ui

HEADERS  +=								\
	$$TOPDIR/lib/core/TSettings.h					\
	$$TOPDIR/lib/widgets/TElidedLabel.h				\
	$$TOPDIR/src/customfilesystemmodel.h				\
	$$TOPDIR/src/widgets/confirmationdialog.h			\
	$$TOPDIR/src/widgets/elidedannotatedlabel.h			\
	$$TOPDIR/src/widgets/filepickerdialog.h				\
	$$TOPDIR/src/widgets/filepickerwidget.h				\
	$$TOPDIR/src/widgets/stoptasksdialog.h				\
	$$TOPDIR/tests/qtest-platform.h

SOURCES += test-small-widgets.cpp					\
	$$TOPDIR/lib/core/TSettings.cpp					\
	$$TOPDIR/lib/widgets/TElidedLabel.cpp				\
	$$TOPDIR/src/customfilesystemmodel.cpp				\
	$$TOPDIR/src/widgets/confirmationdialog.cpp			\
	$$TOPDIR/src/widgets/elidedannotatedlabel.cpp			\
	$$TOPDIR/src/widgets/filepickerdialog.cpp			\
	$$TOPDIR/src/widgets/filepickerwidget.cpp			\
	$$TOPDIR/src/widgets/stoptasksdialog.cpp

include(../tests-include.pri)

test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}";	\
	touch "$${TEST_HOME}/$${TARGET}/empty-file"
