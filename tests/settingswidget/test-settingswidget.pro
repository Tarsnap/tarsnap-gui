TARGET = test-settingswidget
QT = core gui widgets network

# For TarsnapAccount
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

VALGRIND = true

FORMS += ../../forms/settingswidget.ui			\
	../../forms/logindialog.ui

HEADERS  +=						\
	../../lib/core/ConsoleLog.h			\
	../../lib/core/TSettings.h			\
	../../lib/widgets/TElidedLabel.h		\
	../../src/dir-utils.h				\
	../../src/scheduling.h				\
	../../src/tarsnapaccount.h			\
	../../src/tasks/tasks-utils.h			\
	../../src/translator.h				\
	../../src/widgets/confirmationdialog.h		\
	../../src/widgets/schedulingwidgets.h		\
	../../src/widgets/settingswidget.h		\
	../../src/widgets/tarsnapaccountdialog.h	\
	../../tests/qtest-platform.h

SOURCES += test-settingswidget.cpp			\
	../../lib/core/ConsoleLog.cpp			\
	../../lib/core/TSettings.cpp			\
	../../lib/widgets/TElidedLabel.cpp		\
	../../src/dir-utils.cpp				\
	../../src/scheduling.cpp			\
	../../src/tarsnapaccount.cpp			\
	../../src/tasks/tasks-utils.cpp			\
	../../src/translator.cpp			\
	../../src/widgets/confirmationdialog.cpp	\
	../../src/widgets/schedulingwidgets.cpp		\
	../../src/widgets/settingswidget.cpp		\
	../../src/widgets/tarsnapaccountdialog.cpp

include(../tests-include.pro)

test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}";	\
	cp confdir/* "$${TEST_HOME}/$${TARGET}"
