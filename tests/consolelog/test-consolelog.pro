TARGET = test-consolelog
QT = core

HEADERS  +=						\
	../../lib/core/TSettings.h			\
	../../src/debug.h

SOURCES += test-consolelog.cpp				\
	../../lib/core/TSettings.cpp			\
	../../src/debug.cpp

include(../tests-include.pro)

test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}";	\
	mkdir -p "$${TEST_HOME}/$${TARGET}/appdata";			\
	cp -r confdir/* "$${TEST_HOME}/$${TARGET}"
