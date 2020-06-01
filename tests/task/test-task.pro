TARGET = test-task
QT = core

VALGRIND = true

HEADERS  +=						\
	../../lib/core/ConsoleLog.h			\
	../../lib/core/TSettings.h			\
	../../src/basetask.h				\
	../../src/cmdlinetask.h				\
	../../src/tasks/tasks-utils.h			\
	../qtest-platform.h

SOURCES += test-task.cpp				\
	../../lib/core/ConsoleLog.cpp			\
	../../lib/core/TSettings.cpp			\
	../../src/basetask.cpp				\
	../../src/cmdlinetask.cpp			\
	../../src/tasks/tasks-utils.cpp

include(../tests-include.pro)
