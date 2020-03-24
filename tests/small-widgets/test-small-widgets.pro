TARGET = test-small-widgets
QT = core gui widgets

VALGRIND = true

FORMS +=						\
	../../forms/filepickerdialog.ui			\
	../../forms/filepickerwidget.ui			\
	../../forms/stoptasksdialog.ui

HEADERS  +=						\
	../../lib/core/TSettings.h			\
	../../lib/widgets/TElidedLabel.h		\
	../../src/customfilesystemmodel.h		\
	../../src/widgets/confirmationdialog.h		\
	../../src/widgets/elidedannotatedlabel.h	\
	../../src/widgets/filepickerdialog.h		\
	../../src/widgets/filepickerwidget.h		\
	../../src/widgets/stoptasksdialog.h		\
	../../tests/qtest-platform.h

SOURCES += test-small-widgets.cpp			\
	../../lib/core/TSettings.cpp			\
	../../lib/widgets/TElidedLabel.cpp		\
	../../src/customfilesystemmodel.cpp		\
	../../src/widgets/confirmationdialog.cpp	\
	../../src/widgets/elidedannotatedlabel.cpp	\
	../../src/widgets/filepickerdialog.cpp		\
	../../src/widgets/filepickerwidget.cpp		\
	../../src/widgets/stoptasksdialog.cpp

include(../tests-include.pro)

test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}";	\
	touch "$${TEST_HOME}/$${TARGET}/empty-file"
