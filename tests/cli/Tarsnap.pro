TARGET = test-cli
QT = core network sql

VALGRIND = true

# Used in lib/core/optparse.h
VERSION = test
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

SOURCES +=						\
	../../lib/core/ConsoleLog.cpp			\
	../../lib/core/TSettings.cpp			\
	../../lib/util/optparse.c			\
	../../lib/util/optparse_helper.c		\
	../../libcperciva/util/getopt.c			\
	../../libcperciva/util/warnp.c			\
	../../src/app-cmdline.cpp			\
	../../src/backenddata.cpp			\
	../../src/backuptask.cpp			\
	../../src/basetask.cpp				\
	../../src/cmdlinetask.cpp			\
	../../src/filetablemodel.cpp			\
	../../src/humanbytes.cpp			\
	../../src/init-shared.cpp			\
	../../src/jobrunner.cpp				\
	../../src/main.cpp				\
	../../src/parsearchivelistingtask.cpp		\
	../../src/persistentmodel/archive.cpp		\
	../../src/persistentmodel/job.cpp		\
	../../src/persistentmodel/journal.cpp		\
	../../src/persistentmodel/persistentobject.cpp	\
	../../src/persistentmodel/persistentstore.cpp	\
	../../src/persistentmodel/upgrade-store.cpp	\
	../../src/scheduling.cpp			\
	../../src/taskmanager.cpp			\
	../../src/taskqueuer.cpp			\
	../../src/tasks/tasks-misc.cpp			\
	../../src/tasks/tasks-setup.cpp			\
	../../src/tasks/tasks-tarsnap.cpp		\
	../../src/tasks/tasks-utils.cpp			\
	../../src/translator.cpp

HEADERS +=						\
	../../lib/core/ConsoleLog.h			\
	../../lib/core/LogEntry.h			\
	../../lib/core/TSettings.h			\
	../../lib/util/optparse.h			\
	../../lib/util/optparse_helper.h		\
	../../libcperciva/util/getopt.h			\
	../../libcperciva/util/warnp.h			\
	../../src/app-cmdline.h				\
	../../src/backenddata.h				\
	../../src/backuptask.h				\
	../../src/basetask.h				\
	../../src/cmdlinetask.h				\
	../../src/filetablemodel.h			\
	../../src/humanbytes.h				\
	../../src/init-shared.h				\
	../../src/jobrunner.h				\
	../../src/messages/archivefilestat.h		\
	../../src/messages/archiveptr.h			\
	../../src/messages/archiverestoreoptions.h	\
	../../src/messages/backuptaskdataptr.h		\
	../../src/messages/jobptr.h			\
	../../src/messages/notification_info.h		\
	../../src/messages/tarsnaperror.h		\
	../../src/messages/taskstatus.h			\
	../../src/parsearchivelistingtask.h		\
	../../src/persistentmodel/archive.h		\
	../../src/persistentmodel/job.h			\
	../../src/persistentmodel/journal.h		\
	../../src/persistentmodel/persistentobject.h	\
	../../src/persistentmodel/persistentstore.h	\
	../../src/persistentmodel/upgrade-store.h	\
	../../src/scheduling.h				\
	../../src/taskmanager.h				\
	../../src/taskqueuer.h				\
	../../src/tasks/tasks-defs.h			\
	../../src/tasks/tasks-misc.h			\
	../../src/tasks/tasks-setup.h			\
	../../src/tasks/tasks-tarsnap.h			\
	../../src/tasks/tasks-utils.h			\
	../../src/translator.h

RESOURCES += ../../resources/resources-lite.qrc

TRANSLATIONS = resources/translations/tarsnap-gui_en.ts \
               resources/translations/tarsnap-gui_ro.ts


# Include shared settings, but then undo some of them
include(../tests-include.pro)
QT -= testlib
CONFIG -= debug
UI_DIR      = ../../build/cli/
MOC_DIR     = ../../build/cli/
RCC_DIR     = ../../build/cli/
OBJECTS_DIR = ../../build/cli/


CONFDIR ="\"$${TEST_HOME}/Tarsnap Backup Inc./\""
test_home_prep.commands += ; mkdir -p "$${CONFDIR}";		\
	cp confdir/test-cli.conf "$${CONFDIR}/Tarsnap.conf"

test.commands = $${TEST_ENV} ./${TARGET} --check

test_valgrind.commands = $${TEST_ENV} $${VALGRIND_CMD} ./${TARGET} --check
