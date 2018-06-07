QT += testlib core gui widgets sql network
CONFIG += c++11

TEMPLATE = app
TARGET = test-taskmanager

INCLUDEPATH += . ../../src/

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

SOURCES += main.cpp					\
	../../src/persistentmodel/archive.cpp		\
	../../src/persistentmodel/job.cpp		\
	../../src/persistentmodel/persistentobject.cpp	\
	../../src/persistentmodel/persistentstore.cpp	\
	../../src/backuptask.cpp			\
	../../src/debug.cpp				\
	../../src/tarsnaptask.cpp			\
	../../src/utils.cpp				\
	../../src/taskmanager.cpp

TEST_HOME = /tmp/tarsnap-gui-test
DEFINES += "TEST_NAME=\\\"$${TARGET}\\\""
test_home_prep.commands = rm -rf "$${TEST_HOME}/$${TARGET}"

# XDG_*_HOME uses a custom "home" directory to store data, thereby not messing
# up user-wide files.
test.commands =					\
	XDG_CONFIG_HOME=$${TEST_HOME}		\
	XDG_CACHE_HOME=$${TEST_HOME}		\
	XDG_DATA_HOME=$${TEST_HOME}		\
	./${TARGET} -platform offscreen
test.depends = ${TARGET} test_home_prep
QMAKE_EXTRA_TARGETS += test test_home_prep
