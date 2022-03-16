### Compiling

QT += testlib
CONFIG += c++11 debug

TEMPLATE = app

INCLUDEPATH += .				\
	../../lib/core/				\
	../../lib/util/				\
	../../lib/widgets/			\
	../../libcperciva/util/			\
	../../src/


# Pick up extra flags from the environment
QMAKE_CXXFLAGS += $$(CXXFLAGS)
QMAKE_CFLAGS += $$(CFLAGS)
QMAKE_LFLAGS += $$(LDFLAGS)
env_CC = $$(QMAKE_CC)
!isEmpty(env_CC) {
	QMAKE_CC = $$(QMAKE_CC)
}
env_CXX = $$(QMAKE_CXX)
!isEmpty(env_CXX) {
	QMAKE_CXX = $$(QMAKE_CXX)
}
env_LINK = $$(QMAKE_LINK)
!isEmpty(env_LINK) {
	QMAKE_LINK = $$(QMAKE_LINK)
}


### Shared build-dir

UI_DIR      = ../../build/tests/
MOC_DIR     = ../../build/tests/
RCC_DIR     = ../../build/tests/
OBJECTS_DIR = ../../build/tests/


### Stuff for tests

TEST_HOME = /tmp/tarsnap-gui-test
DEFINES += "TEST_DIR=\\\"$${TEST_HOME}/$${TARGET}\\\""
DEFINES += "TEST_NAME=\\\"$${TARGET}\\\""
test_home_prep.commands = rm -rf "$${TEST_HOME}/$${TARGET}"
# XDG_*_HOME uses a custom "home" directory to store data, thereby not messing
# up user-wide files.
TEST_ENV =	XDG_CONFIG_HOME=$${TEST_HOME}		\
		XDG_CACHE_HOME=$${TEST_HOME}		\
		XDG_DATA_HOME=$${TEST_HOME}		\
		XDG_RUNTIME_DIR=$${TEST_HOME}

contains(QT, gui) || contains(QT, widgets) {
	test.commands = $${TEST_ENV} QT_QPA_PLATFORM=offscreen ./${TARGET} \${ONLY}
	test_visual.depends = ${TARGET} test_home_prep
	test_visual.commands = $${TEST_ENV} ./${TARGET} \${ONLY}
} else {
	test.commands = $${TEST_ENV} ./${TARGET} \${ONLY}
}

test.depends = ${TARGET} test_home_prep
QMAKE_EXTRA_TARGETS += test test_visual test_home_prep

!isEmpty(VALGRIND) {
	test_valgrind.depends = ${TARGET} test_home_prep
	QMAKE_EXTRA_TARGETS += test_valgrind

	VALGRIND_SUPPRESSIONS=$$absolute_path("valgrind")/valgrind.supp
	VALGRIND_CMD = "valgrind --leak-check=full --show-leak-kinds=all\
			--suppressions=$${VALGRIND_SUPPRESSIONS}	\
			--gen-suppressions=all				\
			--log-file=valgrind-full.log			\
			--num-callers=40				\
			--leak-check-heuristics=none			\
			--errors-for-leak-kinds=all			\
			--error-exitcode=108"

	contains(QT, gui) || contains(QT, widgets) {
		test_valgrind.commands = $${TEST_ENV} QT_QPA_PLATFORM=offscreen $${VALGRIND_CMD} ./${TARGET} \${ONLY}
	} else {
		test_valgrind.commands = $${TEST_ENV} $${VALGRIND_CMD} ./${TARGET} \${ONLY}
	}
}
