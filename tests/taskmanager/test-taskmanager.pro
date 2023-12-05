TARGET = test-taskmanager
QT = core sql network
TOPDIR = ../..

VALGRIND = true

HEADERS  +=								\
	$$TOPDIR/lib/core/ConsoleLog.h					\
	$$TOPDIR/lib/core/TSettings.h					\
	$$TOPDIR/src/backenddata.h					\
	$$TOPDIR/src/backuptask.h					\
	$$TOPDIR/src/basetask.h						\
	$$TOPDIR/src/cmdlinetask.h					\
	$$TOPDIR/src/dir-utils.h					\
	$$TOPDIR/src/dirinfotask.h					\
	$$TOPDIR/src/humanbytes.h					\
	$$TOPDIR/src/jobrunner.h					\
	$$TOPDIR/src/messages/archivefilestat.h				\
	$$TOPDIR/src/messages/archiveptr.h				\
	$$TOPDIR/src/messages/backuptaskdataptr.h			\
	$$TOPDIR/src/messages/jobptr.h					\
	$$TOPDIR/src/messages/notification_info.h			\
	$$TOPDIR/src/parsearchivelistingtask.h				\
	$$TOPDIR/src/persistentmodel/archive.h				\
	$$TOPDIR/src/persistentmodel/job.h				\
	$$TOPDIR/src/persistentmodel/persistentobject.h			\
	$$TOPDIR/src/persistentmodel/persistentstore.h			\
	$$TOPDIR/src/persistentmodel/upgrade-store.h			\
	$$TOPDIR/src/taskmanager.h					\
	$$TOPDIR/src/taskqueuer.h					\
	$$TOPDIR/src/tasks/tasks-defs.h					\
	$$TOPDIR/src/tasks/tasks-misc.h					\
	$$TOPDIR/src/tasks/tasks-setup.h				\
	$$TOPDIR/src/tasks/tasks-tarsnap.h				\
	$$TOPDIR/src/tasks/tasks-utils.h				\
	../qtest-platform.h

SOURCES += test-taskmanager.cpp						\
	$$TOPDIR/lib/core/ConsoleLog.cpp				\
	$$TOPDIR/lib/core/TSettings.cpp					\
	$$TOPDIR/src/backenddata.cpp					\
	$$TOPDIR/src/backuptask.cpp					\
	$$TOPDIR/src/basetask.cpp					\
	$$TOPDIR/src/cmdlinetask.cpp					\
	$$TOPDIR/src/dir-utils.cpp					\
	$$TOPDIR/src/dirinfotask.cpp					\
	$$TOPDIR/src/humanbytes.cpp					\
	$$TOPDIR/src/jobrunner.cpp					\
	$$TOPDIR/src/parsearchivelistingtask.cpp			\
	$$TOPDIR/src/persistentmodel/archive.cpp			\
	$$TOPDIR/src/persistentmodel/job.cpp				\
	$$TOPDIR/src/persistentmodel/persistentobject.cpp		\
	$$TOPDIR/src/persistentmodel/persistentstore.cpp		\
	$$TOPDIR/src/persistentmodel/upgrade-store.cpp			\
	$$TOPDIR/src/taskmanager.cpp					\
	$$TOPDIR/src/taskqueuer.cpp					\
	$$TOPDIR/src/tasks/tasks-misc.cpp				\
	$$TOPDIR/src/tasks/tasks-setup.cpp				\
	$$TOPDIR/src/tasks/tasks-tarsnap.cpp				\
	$$TOPDIR/src/tasks/tasks-utils.cpp

include(../tests-include.pri)

test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}";	\
	mkdir -p "$${TEST_HOME}/$${TARGET}/cachedir";			\
	touch "$${TEST_HOME}/$${TARGET}/cachedir/stuff"
