TARGET = test-setupwizard
QT = core gui widgets network

VALGRIND = true

FORMS +=						\
	../../forms/setupdialog.ui			\
	../../lib/forms/PathLineBrowse.ui

HEADERS  +=						\
	compare-settings.h				\
	../../lib/core/TSettings.h			\
	../../lib/widgets/ElidedLabel.h			\
	../../lib/widgets/PathLineBrowse.h		\
	../../src/tasks/tasks-utils.h			\
	../../src/utils.h				\
	../../src/widgets/busywidget.h			\
	../../src/widgets/setupdialog.h			\
	../../tests/qtest-platform.h

SOURCES += test-setupwizard.cpp				\
	compare-settings.cpp				\
	../../lib/core/TSettings.cpp			\
	../../lib/widgets/ElidedLabel.cpp		\
	../../lib/widgets/PathLineBrowse.cpp		\
	../../src/tasks/tasks-utils.cpp			\
	../../src/utils.cpp				\
	../../src/widgets/busywidget.cpp		\
	../../src/widgets/setupdialog.cpp

RESOURCES += ../../resources/resources.qrc

include(../tests-include.pro)
