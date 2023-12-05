TARGET = test-cli
QT = core network sql
TOPDIR = ../..

VALGRIND = true

# Used in lib/core/optparse.h
VERSION = test
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# Used in src/init-shared.cpp
DEFINES += TEST_CLI

SOURCES +=								\
	$$TOPDIR/lib/core/ConsoleLog.cpp				\
	$$TOPDIR/lib/core/TSettings.cpp					\
	$$TOPDIR/lib/util/optparse.c					\
	$$TOPDIR/lib/util/optparse_helper.c				\
	$$TOPDIR/libcperciva/util/getopt.c				\
	$$TOPDIR/libcperciva/util/warnp.c				\
	$$TOPDIR/src/app-cmdline.cpp					\
	$$TOPDIR/src/backenddata.cpp					\
	$$TOPDIR/src/backuptask.cpp					\
	$$TOPDIR/src/basetask.cpp					\
	$$TOPDIR/src/cmdlinetask.cpp					\
	$$TOPDIR/src/filetablemodel.cpp					\
	$$TOPDIR/src/humanbytes.cpp					\
	$$TOPDIR/src/init-shared.cpp					\
	$$TOPDIR/src/jobrunner.cpp					\
	$$TOPDIR/src/main.cpp						\
	$$TOPDIR/src/parsearchivelistingtask.cpp			\
	$$TOPDIR/src/persistentmodel/archive.cpp			\
	$$TOPDIR/src/persistentmodel/job.cpp				\
	$$TOPDIR/src/persistentmodel/journal.cpp			\
	$$TOPDIR/src/persistentmodel/persistentobject.cpp		\
	$$TOPDIR/src/persistentmodel/persistentstore.cpp		\
	$$TOPDIR/src/persistentmodel/upgrade-store.cpp			\
	$$TOPDIR/src/scheduling.cpp					\
	$$TOPDIR/src/taskmanager.cpp					\
	$$TOPDIR/src/taskqueuer.cpp					\
	$$TOPDIR/src/tasks/tasks-misc.cpp				\
	$$TOPDIR/src/tasks/tasks-setup.cpp				\
	$$TOPDIR/src/tasks/tasks-tarsnap.cpp				\
	$$TOPDIR/src/tasks/tasks-utils.cpp				\
	$$TOPDIR/src/translator.cpp

HEADERS +=								\
	$$TOPDIR/lib/core/ConsoleLog.h					\
	$$TOPDIR/lib/core/LogEntry.h					\
	$$TOPDIR/lib/core/TSettings.h					\
	$$TOPDIR/lib/util/optparse.h					\
	$$TOPDIR/lib/util/optparse_helper.h				\
	$$TOPDIR/libcperciva/util/getopt.h				\
	$$TOPDIR/libcperciva/util/warnp.h				\
	$$TOPDIR/src/app-cmdline.h					\
	$$TOPDIR/src/backenddata.h					\
	$$TOPDIR/src/backuptask.h					\
	$$TOPDIR/src/basetask.h						\
	$$TOPDIR/src/cmdlinetask.h					\
	$$TOPDIR/src/filetablemodel.h					\
	$$TOPDIR/src/humanbytes.h					\
	$$TOPDIR/src/init-shared.h					\
	$$TOPDIR/src/jobrunner.h					\
	$$TOPDIR/src/messages/archivefilestat.h				\
	$$TOPDIR/src/messages/archiveptr.h				\
	$$TOPDIR/src/messages/archiverestoreoptions.h			\
	$$TOPDIR/src/messages/backuptaskdataptr.h			\
	$$TOPDIR/src/messages/jobptr.h					\
	$$TOPDIR/src/messages/notification_info.h			\
	$$TOPDIR/src/messages/tarsnaperror.h				\
	$$TOPDIR/src/messages/taskstatus.h				\
	$$TOPDIR/src/parsearchivelistingtask.h				\
	$$TOPDIR/src/persistentmodel/archive.h				\
	$$TOPDIR/src/persistentmodel/job.h				\
	$$TOPDIR/src/persistentmodel/journal.h				\
	$$TOPDIR/src/persistentmodel/persistentobject.h			\
	$$TOPDIR/src/persistentmodel/persistentstore.h			\
	$$TOPDIR/src/persistentmodel/upgrade-store.h			\
	$$TOPDIR/src/scheduling.h					\
	$$TOPDIR/src/taskmanager.h					\
	$$TOPDIR/src/taskqueuer.h					\
	$$TOPDIR/src/tasks/tasks-defs.h					\
	$$TOPDIR/src/tasks/tasks-misc.h					\
	$$TOPDIR/src/tasks/tasks-setup.h				\
	$$TOPDIR/src/tasks/tasks-tarsnap.h				\
	$$TOPDIR/src/tasks/tasks-utils.h				\
	$$TOPDIR/src/translator.h

RESOURCES += $$TOPDIR/resources/resources-lite.qrc

TRANSLATIONS = resources/translations/tarsnap-gui_en.ts			\
               resources/translations/tarsnap-gui_ro.ts


# Include shared settings, but then undo some of them
include(../tests-include.pri)
QT -= testlib
CONFIG -= debug
UI_DIR      = $$TOPDIR/build/cli/
MOC_DIR     = $$TOPDIR/build/cli/
RCC_DIR     = $$TOPDIR/build/cli/
OBJECTS_DIR = $$TOPDIR/build/cli/


test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}";	\
	cp -r confdir/* "$${TEST_HOME}/$${TARGET}"

test.commands = $${TEST_ENV} ./${TARGET} --check

test_valgrind.commands = $${TEST_ENV} $${VALGRIND_CMD} ./${TARGET} --check
