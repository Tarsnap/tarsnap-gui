TARGET = test-persistent
QT = core sql

VALGRIND = true

# Needed for the database template
RESOURCES += ../../resources/resources-lite.qrc

HEADERS  +=						\
	../../lib/core/LogEntry.h			\
	../../lib/core/TSettings.h			\
	../../src/messages/archiveptr.h			\
	../../src/persistentmodel/archive.h		\
	../../src/persistentmodel/job.h			\
	../../src/persistentmodel/journal.h		\
	../../src/persistentmodel/persistentobject.h	\
	../../src/persistentmodel/persistentstore.h	\
	../../src/persistentmodel/upgrade-store.h

SOURCES += test-persistent.cpp				\
	../../lib/core/TSettings.cpp			\
	../../src/persistentmodel/archive.cpp		\
	../../src/persistentmodel/job.cpp		\
	../../src/persistentmodel/journal.cpp		\
	../../src/persistentmodel/persistentobject.cpp	\
	../../src/persistentmodel/persistentstore.cpp	\
	../../src/persistentmodel/upgrade-store.cpp

include(../tests-include.pro)

test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}";	\
	cp -r confdir/* "$${TEST_HOME}/$${TARGET}"
