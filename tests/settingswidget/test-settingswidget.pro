TARGET = test-settingswidget
QT = core gui widgets network
TOPDIR = ../..

# For TarsnapAccount
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

VALGRIND = true

FORMS += $$TOPDIR/forms/settingswidget.ui				\
	$$TOPDIR/forms/logindialog.ui

HEADERS  +=								\
	$$TOPDIR/lib/core/ConsoleLog.h					\
	$$TOPDIR/lib/core/TSettings.h					\
	$$TOPDIR/lib/widgets/TElidedLabel.h				\
	$$TOPDIR/src/dir-utils.h					\
	$$TOPDIR/src/scheduling.h					\
	$$TOPDIR/src/tarsnapaccount.h					\
	$$TOPDIR/src/tasks/tasks-utils.h				\
	$$TOPDIR/src/translator.h					\
	$$TOPDIR/src/widgets/confirmationdialog.h			\
	$$TOPDIR/src/widgets/schedulingwidgets.h			\
	$$TOPDIR/src/widgets/settingswidget.h				\
	$$TOPDIR/src/widgets/tarsnapaccountdialog.h			\
	$$TOPDIR/tests/qtest-platform.h

SOURCES += test-settingswidget.cpp					\
	$$TOPDIR/lib/core/ConsoleLog.cpp				\
	$$TOPDIR/lib/core/TSettings.cpp					\
	$$TOPDIR/lib/widgets/TElidedLabel.cpp				\
	$$TOPDIR/src/dir-utils.cpp					\
	$$TOPDIR/src/scheduling.cpp					\
	$$TOPDIR/src/tarsnapaccount.cpp					\
	$$TOPDIR/src/tasks/tasks-utils.cpp				\
	$$TOPDIR/src/translator.cpp					\
	$$TOPDIR/src/widgets/confirmationdialog.cpp			\
	$$TOPDIR/src/widgets/schedulingwidgets.cpp			\
	$$TOPDIR/src/widgets/settingswidget.cpp				\
	$$TOPDIR/src/widgets/tarsnapaccountdialog.cpp

include(../tests-include.pri)

test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}";	\
	cp confdir/* "$${TEST_HOME}/$${TARGET}"
