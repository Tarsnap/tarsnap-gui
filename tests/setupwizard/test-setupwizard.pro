TARGET = test-setupwizard
QT = core gui widgets network

VALGRIND = true

FORMS += ../../forms/setupdialog.ui

HEADERS  +=						\
	../../lib/core/TSettings.h			\
	../../src/tasks/tasks-utils.h			\
	../../src/utils.h				\
	../../src/widgets/elidedlabel.h			\
	../../src/widgets/setupdialog.h			\
	../../tests/qtest-platform.h

SOURCES += test-setupwizard.cpp				\
	../../lib/core/TSettings.cpp			\
	../../src/tasks/tasks-utils.cpp			\
	../../src/utils.cpp				\
	../../src/widgets/elidedlabel.cpp		\
	../../src/widgets/setupdialog.cpp

include(../tests-include.pro)
