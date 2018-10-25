QT = core network sql
CONFIG += c++11

TEMPLATE = app
TARGET = tarsnap-gui-cli

# Used in lib/core/optparse.h
VERSION = test
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

SOURCES +=						\
	../../lib/core/TSettings.cpp			\
	../../lib/util/optparse.c			\
	../../lib/util/optparse_helper.c		\
	../../libcperciva/util/getopt.c			\
	../../libcperciva/util/warnp.c			\
	../../src/app-cmdline.cpp			\
	../../src/backuptask.cpp			\
	../../src/debug.cpp				\
	../../src/filetablemodel.cpp			\
	../../src/init-shared.cpp			\
	../../src/main.cpp				\
	../../src/persistentmodel/archive.cpp		\
	../../src/persistentmodel/job.cpp		\
	../../src/persistentmodel/journal.cpp		\
	../../src/persistentmodel/persistentobject.cpp	\
	../../src/persistentmodel/persistentstore.cpp	\
	../../src/scheduling.cpp			\
	../../src/tarsnaptask.cpp			\
	../../src/taskmanager.cpp			\
	../../src/translator.cpp			\
	../../src/utils.cpp

HEADERS +=						\
	../../lib/core/TSettings.h			\
	../../lib/util/optparse.h			\
	../../lib/util/optparse_helper.h		\
	../../libcperciva/util/getopt.h			\
	../../libcperciva/util/warnp.h			\
	../../src/app-cmdline.h				\
	../../src/backuptask.h				\
	../../src/debug.h				\
	../../src/filetablemodel.h			\
	../../src/init-shared.h				\
	../../src/persistentmodel/archive.h		\
	../../src/persistentmodel/job.h			\
	../../src/persistentmodel/journal.h		\
	../../src/persistentmodel/persistentobject.h	\
	../../src/persistentmodel/persistentstore.h	\
	../../src/scheduling.h				\
	../../src/tarsnaperror.h			\
	../../src/tarsnaptask.h				\
	../../src/taskmanager.h				\
	../../src/taskstatus.h				\
	../../src/translator.h				\
	../../src/utils.h

INCLUDEPATH += \
            += ../../libcperciva/util/ \
            += ../../lib/core/ \
            += ../../lib/util/ \
            += ../../src/

RESOURCES += ../../resources/resources-lite.qrc

TRANSLATIONS = resources/translations/tarsnap-gui_en.ts \
               resources/translations/tarsnap-gui_ro.ts


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


# Cleaner source directory
UI_DIR      = ../../build/cli/
MOC_DIR     = ../../build/cli/
RCC_DIR     = ../../build/cli/
OBJECTS_DIR = ../../build/cli/


# Valgrind
test_valgrind.depends = ${TARGET}
QMAKE_EXTRA_TARGETS += test_valgrind

VALGRIND_SUPPRESSIONS=$$absolute_path("../valgrind")/valgrind.supp
VALGRIND_CMD = "valgrind --leak-check=full --show-leak-kinds=all\
		--suppressions=$${VALGRIND_SUPPRESSIONS}	\
		--gen-suppressions=all				\
		--log-file=valgrind-full.log			\
		--error-exitcode=108"

test_valgrind.commands = $${VALGRIND_CMD} ./${TARGET} --check

# Do not use the shared tests .pro file.
