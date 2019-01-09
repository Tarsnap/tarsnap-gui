TARGET = test-cfsm
QT = core widgets

VALGRIND = true

HEADERS	+=						\
	../../src/customfilesystemmodel.h		\
	scenario-num.h					\
	run-scenario.h

SOURCES	+= test-cfsm.cpp				\
	../../src/customfilesystemmodel.cpp		\
	run-scenario.cpp

include(../tests-include.pro)

test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}";	\
	cp -r dirs/* "$${TEST_HOME}/$${TARGET}"
