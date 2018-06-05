QT += testlib core gui widgets sql network
CONFIG += c++11

TEMPLATE = app
TARGET = test-setupwizard

INCLUDEPATH += . ../../src/ ../../src/widgets/

FORMS += ../../forms/setupdialog.ui

HEADERS  +=						\
	../../src/utils.h				\
	../../src/widgets/elidedlabel.h			\
	../../src/widgets/setupdialog.h

SOURCES += main.cpp					\
	../../src/utils.cpp				\
	../../src/widgets/elidedlabel.cpp		\
	../../src/widgets/setupdialog.cpp

TEST_HOME = /tmp/tarsnap-gui-test
DEFINES += "TEST_NAME=\\\"$${TARGET}\\\""
test_home_prep.commands = rm -rf "$${TEST_HOME}/$${TARGET}"

# XDG_*_HOME uses a custom "home" directory to store data, thereby not messing
# up user-wide files.
test.commands =					\
	XDG_CONFIG_HOME=$${TEST_HOME}		\
	XDG_CACHE_HOME=$${TEST_HOME}		\
	XDG_DATA_HOME=$${TEST_HOME}		\
	./${TARGET}
test.depends = ${TARGET} test_home_prep
QMAKE_EXTRA_TARGETS += test test_home_prep
