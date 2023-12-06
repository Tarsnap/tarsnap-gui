TARGET = test-lib-core
QT = core
TOPDIR = ../..

VALGRIND = true

HEADERS  +=								\
	$$TOPDIR/lib/core/TSettings.h

SOURCES += test-lib-core.cpp						\
	$$TOPDIR/lib/core/TSettings.cpp

include(../tests-include.pri)

test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}";	\
	cp confdir/*.conf "$${TEST_HOME}/$${TARGET}"
