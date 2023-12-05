TARGET = test-persistent
QT = core sql
TOPDIR = ../..

VALGRIND = true

# Needed for the database template
RESOURCES += $$TOPDIR/resources/resources-lite.qrc

HEADERS  +=								\
	$$TOPDIR/lib/core/LogEntry.h					\
	$$TOPDIR/lib/core/TSettings.h					\
	$$TOPDIR/src/messages/archiveptr.h				\
	$$TOPDIR/src/persistentmodel/archive.h				\
	$$TOPDIR/src/persistentmodel/job.h				\
	$$TOPDIR/src/persistentmodel/journal.h				\
	$$TOPDIR/src/persistentmodel/persistentobject.h			\
	$$TOPDIR/src/persistentmodel/persistentstore.h			\
	$$TOPDIR/src/persistentmodel/upgrade-store.h

SOURCES += test-persistent.cpp						\
	$$TOPDIR/lib/core/TSettings.cpp					\
	$$TOPDIR/src/persistentmodel/archive.cpp			\
	$$TOPDIR/src/persistentmodel/job.cpp				\
	$$TOPDIR/src/persistentmodel/journal.cpp			\
	$$TOPDIR/src/persistentmodel/persistentobject.cpp		\
	$$TOPDIR/src/persistentmodel/persistentstore.cpp		\
	$$TOPDIR/src/persistentmodel/upgrade-store.cpp

include(../tests-include.pri)

test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}";	\
	cp -r confdir/* "$${TEST_HOME}/$${TARGET}"
