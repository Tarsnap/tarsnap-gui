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

# Cleaner source directory
UI_DIR      = ../../build/cli/
MOC_DIR     = ../../build/cli/
RCC_DIR     = ../../build/cli/
OBJECTS_DIR = ../../build/cli/

# Do not use the shared tests .pro file.
