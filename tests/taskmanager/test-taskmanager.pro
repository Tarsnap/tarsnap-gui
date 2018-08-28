TARGET = test-taskmanager

HEADERS  +=						\
	../../src/persistentmodel/archive.h		\
	../../src/persistentmodel/job.h			\
	../../src/persistentmodel/persistentobject.h	\
	../../src/persistentmodel/persistentstore.h	\
	../../src/backuptask.h				\
	../../src/debug.h				\
	../../src/tarsnaptask.h				\
	../../src/utils.h				\
	../../src/taskmanager.h

SOURCES += test-taskmanager.cpp				\
	../../src/persistentmodel/archive.cpp		\
	../../src/persistentmodel/job.cpp		\
	../../src/persistentmodel/persistentobject.cpp	\
	../../src/persistentmodel/persistentstore.cpp	\
	../../src/backuptask.cpp			\
	../../src/debug.cpp				\
	../../src/tarsnaptask.cpp			\
	../../src/utils.cpp				\
	../../src/taskmanager.cpp

include(../tests-include.pro)
