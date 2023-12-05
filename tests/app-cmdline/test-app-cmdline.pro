TARGET = test-app-cmdline
QT = core network sql
TOPDIR = ../..

VALGRIND = true

# Used in lib/core/optparse.h
VERSION = test
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

SOURCES += test-app-cmdline.cpp						\
	$$TOPDIR/lib/core/ConsoleLog.cpp				\
	$$TOPDIR/lib/core/TSettings.cpp					\
	$$TOPDIR/lib/util/optparse.c					\
	$$TOPDIR/lib/util/optparse_helper.c				\
	$$TOPDIR/libcperciva/util/getopt.c				\
	$$TOPDIR/libcperciva/util/warnp.c				\
	$$TOPDIR/src/app-cmdline.cpp					\
	$$TOPDIR/src/backuptask.cpp					\
	$$TOPDIR/src/basetask.cpp					\
	$$TOPDIR/src/init-shared.cpp					\
	$$TOPDIR/src/parsearchivelistingtask.cpp			\
	$$TOPDIR/src/persistentmodel/archive.cpp			\
	$$TOPDIR/src/persistentmodel/job.cpp				\
	$$TOPDIR/src/persistentmodel/journal.cpp			\
	$$TOPDIR/src/persistentmodel/persistentobject.cpp		\
	$$TOPDIR/src/persistentmodel/persistentstore.cpp		\
	$$TOPDIR/src/persistentmodel/upgrade-store.cpp			\
	$$TOPDIR/src/scheduling.cpp					\
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
	$$TOPDIR/src/backuptask.h					\
	$$TOPDIR/src/basetask.h						\
	$$TOPDIR/src/init-shared.h					\
	$$TOPDIR/src/messages/archivefilestat.h				\
	$$TOPDIR/src/messages/archiveptr.h				\
	$$TOPDIR/src/messages/archiverestoreoptions.h			\
	$$TOPDIR/src/messages/backuptaskdataptr.h			\
	$$TOPDIR/src/messages/jobptr.h					\
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
	$$TOPDIR/src/tasks/tasks-utils.h				\
	$$TOPDIR/src/translator.h					\
	../qtest-platform.h

RESOURCES += $$TOPDIR/resources/resources-lite.qrc

TRANSLATIONS = resources/translations/tarsnap-gui_en.ts			\
               resources/translations/tarsnap-gui_ro.ts

include(../tests-include.pri)

test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}";	\
	cp confdir/test-app-cmdline.conf "$${TEST_HOME}/$${TARGET}"
