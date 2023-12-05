TARGET = test-consolelog
QT = core
TOPDIR = ../..

VALGRIND = true

HEADERS  +=								\
	$$TOPDIR/lib/core/ConsoleLog.h					\
	$$TOPDIR/lib/core/TSettings.h

SOURCES += test-consolelog.cpp						\
	$$TOPDIR/lib/core/ConsoleLog.cpp				\
	$$TOPDIR/lib/core/TSettings.cpp

include(../tests-include.pri)

test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}";	\
	mkdir -p "$${TEST_HOME}/$${TARGET}/appdata";			\
	cp -r confdir/* "$${TEST_HOME}/$${TARGET}"
