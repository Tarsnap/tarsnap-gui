TARGET = test-app-cmdline
QT = core network sql

VALGRIND = true

# Used in lib/core/optparse.h
VERSION = test
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

SOURCES += test-app-cmdline.cpp				\
	../../lib/core/ConsoleLog.cpp			\
	../../lib/core/TSettings.cpp			\
	../../lib/util/optparse.c			\
	../../lib/util/optparse_helper.c		\
	../../libcperciva/util/getopt.c			\
	../../libcperciva/util/warnp.c			\
	../../src/app-cmdline.cpp			\
	../../src/backuptask.cpp			\
	../../src/basetask.cpp				\
	../../src/init-shared.cpp			\
	../../src/parsearchivelistingtask.cpp		\
	../../src/persistentmodel/archive.cpp		\
	../../src/persistentmodel/job.cpp		\
	../../src/persistentmodel/journal.cpp		\
	../../src/persistentmodel/persistentobject.cpp	\
	../../src/persistentmodel/persistentstore.cpp	\
	../../src/persistentmodel/upgrade-store.cpp	\
	../../src/scheduling.cpp			\
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
	../../src/backuptask.h				\
	../../src/basetask.h				\
	../../src/init-shared.h				\
	../../src/messages/archivefilestat.h		\
	../../src/messages/archiveptr.h			\
	../../src/messages/archiverestoreoptions.h	\
	../../src/messages/backuptaskdataptr.h		\
	../../src/messages/jobptr.h			\
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
	../../src/tasks/tasks-utils.h			\
	../../src/translator.h				\
	../qtest-platform.h

RESOURCES += ../../resources/resources-lite.qrc

TRANSLATIONS = resources/translations/tarsnap-gui_en.ts \
               resources/translations/tarsnap-gui_ro.ts

include(../tests-include.pro)

test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}";	\
	cp confdir/test-app-cmdline.conf "$${TEST_HOME}/$${TARGET}"
