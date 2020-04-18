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
	../../src/scheduling.cpp			\
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
	../../src/archivefilestat.h			\
	../../src/archiverestoreoptions.h		\
	../../src/backuptask.h				\
	../../src/backuptaskdataptr.h			\
	../../src/basetask.h				\
	../../src/init-shared.h				\
	../../src/parsearchivelistingtask.h		\
	../../src/persistentmodel/archive.h		\
	../../src/persistentmodel/archiveptr.h		\
	../../src/persistentmodel/job.h			\
	../../src/messages/jobptr.h			\
	../../src/persistentmodel/journal.h		\
	../../src/persistentmodel/persistentobject.h	\
	../../src/persistentmodel/persistentstore.h	\
	../../src/scheduling.h				\
	../../src/tarsnaperror.h			\
	../../src/tasks/tasks-utils.h			\
	../../src/taskstatus.h				\
	../../src/translator.h				\
	../../src/utils.h				\
	../qtest-platform.h

RESOURCES += ../../resources/resources-lite.qrc

TRANSLATIONS = resources/translations/tarsnap-gui_en.ts \
               resources/translations/tarsnap-gui_ro.ts

include(../tests-include.pro)

test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}";	\
	cp confdir/test-app-cmdline.conf "$${TEST_HOME}/$${TARGET}"
