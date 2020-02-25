TARGET = test-small-widgets
QT = core gui widgets

VALGRIND = true

FORMS +=						\
	../../forms/filepickerdialog.ui			\
	../../forms/filepickerwidget.ui			\
	../../lib/forms/TWizard.ui

RESOURCES += ../../resources/resources.qrc

HEADERS  +=						\
	../../lib/core/TSettings.h			\
	../../lib/widgets/TElidedLabel.h		\
	../../lib/widgets/TWizard.h			\
	../../lib/widgets/TWizardPage.h			\
	../../src/customfilesystemmodel.h		\
	../../src/widgets/busywidget.h			\
	../../src/widgets/confirmationdialog.h		\
	../../src/widgets/elidedannotatedlabel.h	\
	../../src/widgets/filepickerdialog.h		\
	../../src/widgets/filepickerwidget.h		\
	../../tests/qtest-platform.h

SOURCES += test-small-widgets.cpp			\
	../../lib/core/TSettings.cpp			\
	../../lib/widgets/TElidedLabel.cpp		\
	../../lib/widgets/TWizard.cpp			\
	../../lib/widgets/TWizardPage.cpp		\
	../../src/customfilesystemmodel.cpp		\
	../../src/widgets/busywidget.cpp		\
	../../src/widgets/confirmationdialog.cpp	\
	../../src/widgets/elidedannotatedlabel.cpp	\
	../../src/widgets/filepickerdialog.cpp		\
	../../src/widgets/filepickerwidget.cpp

include(../tests-include.pro)

test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}";	\
	touch "$${TEST_HOME}/$${TARGET}/empty-file"
