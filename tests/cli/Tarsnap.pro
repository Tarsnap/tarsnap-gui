TARGET = test-cli
QT = core network sql

VALGRIND = true

# Used in lib/core/optparse.h
VERSION = test
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

SOURCES +=						\
	../../lib/core/TSettings.cpp			\
	../../lib/util/optparse.c			\
	../../lib/util/optparse_helper.c		\
	../../libcperciva/util/getopt.c			\
	../../libcperciva/util/warnp.c			\
	../../src/app-cmdline.cpp			\
	../../src/backuptask.cpp			\
	../../src/debug.cpp				\
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
	../../src/translator.cpp			\
	../../src/utils.cpp

HEADERS +=						\
	../../lib/core/TSettings.h			\
	../../lib/util/optparse.h			\
	../../lib/util/optparse_helper.h		\
	../../libcperciva/util/getopt.h			\
	../../libcperciva/util/warnp.h			\
	../../src/app-cmdline.h				\
	../../src/backuptask.h				\
	../../src/debug.h				\
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
	../../src/taskstatus.h				\
	../../src/translator.h				\
	../../src/utils.h

RESOURCES += ../../resources/resources-lite.qrc

TRANSLATIONS = resources/translations/tarsnap-gui_en.ts \
               resources/translations/tarsnap-gui_ro.ts

include(../tests-include.pro)

QT -= testlib
CONFIG -= debug

CONFDIR ="\"$${TEST_HOME}/Tarsnap Backup Inc./\""
test_home_prep.commands += ; mkdir -p "$${CONFDIR}";		\
	cp confdir/test-cli.conf "$${CONFDIR}/Tarsnap.conf"

test.commands = $${TEST_ENV} ./${TARGET} --check

test_valgrind.commands = $${TEST_ENV} $${VALGRIND_CMD} ./${TARGET} --check
