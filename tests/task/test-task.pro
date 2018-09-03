TARGET = test-task

HEADERS  +=						\
	../../src/debug.h				\
	../../src/tarsnaptask.h				\
	../../src/utils.h

SOURCES += test-task.cpp				\
	../../src/debug.cpp				\
	../../src/tarsnaptask.cpp			\
	../../src/utils.cpp

include(../tests-include.pro)
