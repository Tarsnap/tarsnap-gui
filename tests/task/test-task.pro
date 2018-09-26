TARGET = test-task
QT = core

HEADERS  +=						\
	../../lib/core/TSettings.h			\
	../../src/debug.h				\
	../../src/tarsnaptask.h				\
	../../src/utils.h

SOURCES += test-task.cpp				\
	../../lib/core/TSettings.cpp			\
	../../src/debug.cpp				\
	../../src/tarsnaptask.cpp			\
	../../src/utils.cpp

include(../tests-include.pro)
