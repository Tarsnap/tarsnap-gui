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
	../../src/backuptask.cpp			\
	../../src/filetablemodel.cpp			\
	../../src/init-shared.cpp			\
	../../src/main.cpp				\
	../../src/persistentmodel/archive.cpp		\
	../../src/persistentmodel/job.cpp		\
	../../src/persistentmodel/journal.cpp		\
	../../src/persistentmodel/persistentobject.cpp	\
	../../src/persistentmodel/persistentstore.cpp	\
	../../src/scheduling.cpp			\
	../../src/tarsnaptask.cpp			\
	../../src/taskmanager.cpp			\
	../../src/tasks/tasks-misc.cpp			\
	../../src/tasks/tasks-setup.cpp			\
	../../src/tasks/tasks-tarsnap.cpp		\
	../../src/tasks/tasks-utils.cpp			\
	../../src/translator.cpp			\
	../../src/utils.cpp

HEADERS +=						\
	../../lib/core/ConsoleLog.h			\
	../../lib/core/LogEntry.h			\
	../../lib/core/TSettings.h			\
	../../lib/util/optparse.h			\
	../../lib/util/optparse_helper.h		\
	../../libcperciva/util/getopt.h			\
	../../libcperciva/util/warnp.h			\
	../../src/app-cmdline.h				\
	../../src/backuptask.h				\
	../../src/filetablemodel.h			\
	../../src/init-shared.h				\
	../../src/persistentmodel/archive.h		\
	../../src/persistentmodel/job.h			\
	../../src/persistentmodel/journal.h		\
	../../src/persistentmodel/persistentobject.h	\
	../../src/persistentmodel/persistentstore.h	\
	../../src/scheduling.h				\
	../../src/tarsnaperror.h			\
	../../src/tarsnaptask.h				\
	../../src/taskmanager.h				\
	../../src/tasks/tasks-defs.h			\
	../../src/tasks/tasks-misc.h			\
	../../src/tasks/tasks-setup.h			\
	../../src/tasks/tasks-tarsnap.h			\
	../../src/tasks/tasks-utils.h			\
	../../src/taskstatus.h				\
	../../src/translator.h				\
	../../src/utils.h

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
