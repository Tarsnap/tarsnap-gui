TARGET = test-core
QT = core
TOPDIR = ../..

VALGRIND = true

HEADERS  +=								\
	../../lib/core/TSettings.h

SOURCES += test-core.cpp						\
	../../lib/core/TSettings.cpp					\

include(../tests-include.pri)

test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}";	\
	cp confdir/*.conf "$${TEST_HOME}/$${TARGET}"
