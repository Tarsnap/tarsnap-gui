TARGET = test-taskmanager
QT = core sql network

VALGRIND = true

HEADERS  +=						\
	../../lib/core/ConsoleLog.h			\
	../../lib/core/TSettings.h			\
	../../src/backuptask.h				\
	../../src/persistentmodel/archive.h		\
	../../src/persistentmodel/job.h			\
	../../src/persistentmodel/persistentobject.h	\
	../../src/persistentmodel/persistentstore.h	\
	../../src/tarsnaptask.h				\
	../../src/taskmanager.h				\
	../../src/tasks/tasks-defs.h			\
	../../src/tasks/tasks-misc.h			\
	../../src/tasks/tasks-setup.h			\
	../../src/tasks/tasks-tarsnap.h			\
	../../src/tasks/tasks-utils.h			\
	../../src/utils.h				\
	../qtest-platform.h

SOURCES += test-taskmanager.cpp				\
	../../lib/core/ConsoleLog.cpp			\
	../../lib/core/TSettings.cpp			\
	../../src/backuptask.cpp			\
	../../src/persistentmodel/archive.cpp		\
	../../src/persistentmodel/job.cpp		\
	../../src/persistentmodel/persistentobject.cpp	\
	../../src/persistentmodel/persistentstore.cpp	\
	../../src/tarsnaptask.cpp			\
	../../src/taskmanager.cpp			\
	../../src/tasks/tasks-misc.cpp			\
	../../src/tasks/tasks-setup.cpp			\
	../../src/tasks/tasks-tarsnap.cpp		\
	../../src/tasks/tasks-utils.cpp			\
	../../src/utils.cpp

include(../tests-include.pro)

test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}";	\
	mkdir -p "$${TEST_HOME}/$${TARGET}/cachedir";			\
	touch "$${TEST_HOME}/$${TARGET}/cachedir/stuff"
