TARGET = test-taskmanager

HEADERS  +=						\
	../../src/backuptask.h				\
	../../src/debug.h				\
	../../src/persistentmodel/archive.h		\
	../../src/persistentmodel/job.h			\
	../../src/persistentmodel/persistentobject.h	\
	../../src/persistentmodel/persistentstore.h	\
	../../src/tarsnaptask.h				\
	../../src/taskmanager.h				\
	../../src/utils.h

SOURCES += test-taskmanager.cpp				\
	../../src/backuptask.cpp			\
	../../src/debug.cpp				\
	../../src/persistentmodel/archive.cpp		\
	../../src/persistentmodel/job.cpp		\
	../../src/persistentmodel/persistentobject.cpp	\
	../../src/persistentmodel/persistentstore.cpp	\
	../../src/tarsnaptask.cpp			\
	../../src/taskmanager.cpp			\
	../../src/utils.cpp

include(../tests-include.pro)
