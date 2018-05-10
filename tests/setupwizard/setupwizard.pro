QT += testlib core gui widgets sql network
CONFIG += c++11

TEMPLATE = app
TARGET = setupwizard

INCLUDEPATH += . ../../src/ ../../src/widgets/

FORMS += ../../forms/setupdialog.ui

HEADERS  +=						\
	../../src/utils.h				\
	../../src/widgets/elidedlabel.h			\
	../../src/widgets/setupdialog.h

SOURCES += testsetupwizard.cpp				\
	../../src/utils.cpp				\
	../../src/widgets/elidedlabel.cpp		\
	../../src/widgets/setupdialog.cpp

# XDG_CONFIG_HOME allows use to create a temporary config file in this
# directory, without messing up the user-wide config file.
# HACK: removing the target is an ugly hack to imitate .PHONY in the Makefile.
test.commands = XDG_CONFIG_HOME=. ./${TARGET} ; rm -f ${TARGET}
test.depends = ${TARGET}
QMAKE_EXTRA_TARGETS += test
