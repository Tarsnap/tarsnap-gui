TARGET = test-task

HEADERS  +=						\
	../../src/debug.h				\
	../../src/utils.h				\
	../../src/tarsnaptask.h

SOURCES += test-task.cpp				\
	../../src/debug.cpp				\
	../../src/utils.cpp				\
	../../src/tarsnaptask.cpp

include(../tests-include.pro)
