### Compiling

QT += testlib
CONFIG += c++11 debug

TEMPLATE = app

INCLUDEPATH += . ../../src/ ../../src/widgets/ ../../lib/core/


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
	test.commands = $${TEST_ENV} ./${TARGET} -platform offscreen
	test_visual.depends = ${TARGET} test_home_prep
	test_visual.commands = $${TEST_ENV} ./${TARGET}
} else {
	test.commands = $${TEST_ENV} ./${TARGET}
}

test.depends = ${TARGET} test_home_prep
QMAKE_EXTRA_TARGETS += test test_visual test_home_prep
