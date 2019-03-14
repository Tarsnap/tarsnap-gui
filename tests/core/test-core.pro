TARGET = test-core
QT = core

VALGRIND = true

HEADERS  +=						\
	../../lib/core/TSettings.h

SOURCES += test-core.cpp				\
	../../lib/core/TSettings.cpp			\

include(../tests-include.pro)

test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}";	\
	cp confdir/*.conf "$${TEST_HOME}/$${TARGET}"
