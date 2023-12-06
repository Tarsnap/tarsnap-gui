TARGET = test-lib-core
QT = core
TOPDIR = ../..

VALGRIND = true

HEADERS  +=								\
	$$TOPDIR/lib/core/ConsoleLog.h					\
	$$TOPDIR/lib/core/TSettings.h

SOURCES += test-lib-core.cpp						\
	$$TOPDIR/lib/core/ConsoleLog.cpp				\
	$$TOPDIR/lib/core/TSettings.cpp

include(../tests-include.pri)

# We need the extra "appdata" to save the logfile.
test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}";	\
	mkdir -p "$${TEST_HOME}/$${TARGET}/appdata";			\
	cp confdir/*.conf "$${TEST_HOME}/$${TARGET}"
