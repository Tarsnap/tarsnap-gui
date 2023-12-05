TARGET = test-task
QT = core
TOPDIR = ../..

VALGRIND = true

HEADERS  +=								\
	$$TOPDIR/lib/core/ConsoleLog.h					\
	$$TOPDIR/lib/core/TSettings.h					\
	$$TOPDIR/src/basetask.h						\
	$$TOPDIR/src/cmdlinetask.h					\
	$$TOPDIR/src/dir-utils.h					\
	$$TOPDIR/src/tasks/tasks-utils.h				\
	../qtest-platform.h

SOURCES += test-task.cpp						\
	$$TOPDIR/lib/core/ConsoleLog.cpp				\
	$$TOPDIR/lib/core/TSettings.cpp					\
	$$TOPDIR/src/basetask.cpp					\
	$$TOPDIR/src/cmdlinetask.cpp					\
	$$TOPDIR/src/dir-utils.cpp					\
	$$TOPDIR/src/tasks/tasks-utils.cpp

include(../tests-include.pri)
