TARGET = test-app-setup
QT = core gui widgets network sql

VALGRIND = true

# Used in lib/core/optparse.h
VERSION = test
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

FORMS +=						\
	../../forms/setupwizard_cli.ui			\
	../../forms/setupwizard_final.ui		\
	../../forms/setupwizard_intro.ui		\
	../../forms/setupwizard_register.ui		\
	../../lib/forms/PathComboBrowse.ui		\
	../../lib/forms/PathLineBrowse.ui		\
	../../lib/forms/TWizard.ui

SOURCES += test-app-setup.cpp				\
	../../lib/core/ConsoleLog.cpp			\
	../../lib/core/TSettings.cpp			\
	../../lib/util/optparse.c			\
	../../lib/util/optparse_helper.c		\
	../../lib/widgets/ElidedLabel.cpp		\
	../../lib/widgets/OkLabel.cpp			\
	../../lib/widgets/PathComboBrowse.cpp		\
	../../lib/widgets/PathLineBrowse.cpp		\
	../../lib/widgets/TWizard.cpp			\
	../../lib/widgets/TWizardPage.cpp		\
	../../libcperciva/util/getopt.c			\
	../../libcperciva/util/warnp.c			\
	../../src/app-cmdline.cpp			\
	../../src/app-setup.cpp				\
	../../src/backuptask.cpp			\
	../../src/filetablemodel.cpp			\
	../../src/init-shared.cpp			\
	../../src/persistentmodel/archive.cpp		\
	../../src/persistentmodel/job.cpp		\
	../../src/persistentmodel/journal.cpp		\
	../../src/persistentmodel/persistentobject.cpp	\
	../../src/persistentmodel/persistentstore.cpp	\
	../../src/scheduling.cpp			\
	../../src/setupwizard/setupwizard.cpp		\
	../../src/setupwizard/setupwizard_cli.cpp	\
	../../src/setupwizard/setupwizard_final.cpp	\
	../../src/setupwizard/setupwizard_intro.cpp	\
	../../src/setupwizard/setupwizard_register.cpp	\
	../../src/tarsnaptask.cpp			\
	../../src/taskmanager.cpp			\
	../../src/tasks/tasks-setup.cpp			\
	../../src/tasks/tasks-utils.cpp			\
	../../src/translator.cpp			\
	../../src/utils.cpp				\
	../../src/widgets/busywidget.cpp

HEADERS +=						\
	../../lib/core/ConsoleLog.h			\
	../../lib/core/TSettings.h			\
	../../lib/util/optparse.h			\
	../../lib/util/optparse_helper.h		\
	../../lib/widgets/ElidedLabel.h			\
	../../lib/widgets/OkLabel.h			\
	../../lib/widgets/PathComboBrowse.h		\
	../../lib/widgets/PathLineBrowse.h		\
	../../lib/widgets/TWizard.h			\
	../../lib/widgets/TWizardPage.h			\
	../../libcperciva/util/getopt.h			\
	../../libcperciva/util/warnp.h			\
	../../src/app-cmdline.h				\
	../../src/app-setup.h				\
	../../src/backuptask.h				\
	../../src/filetablemodel.h			\
	../../src/init-shared.h				\
	../../src/persistentmodel/archive.h		\
	../../src/persistentmodel/job.h			\
	../../src/persistentmodel/journal.h		\
	../../src/persistentmodel/persistentobject.h	\
	../../src/persistentmodel/persistentstore.h	\
	../../src/scheduling.h				\
	../../src/setupwizard/setupwizard.h		\
	../../src/setupwizard/setupwizard_cli.h		\
	../../src/setupwizard/setupwizard_final.h	\
	../../src/setupwizard/setupwizard_intro.h	\
	../../src/setupwizard/setupwizard_register.h	\
	../../src/tarsnaperror.h			\
	../../src/tarsnaptask.h				\
	../../src/taskmanager.h				\
	../../src/tasks/tasks-defs.h			\
	../../src/tasks/tasks-setup.h			\
	../../src/tasks/tasks-utils.h			\
	../../src/taskstatus.h				\
	../../src/translator.h				\
	../../src/utils.h				\
	../../src/widgets/busywidget.h

RESOURCES += ../../resources/resources.qrc

TRANSLATIONS = resources/translations/tarsnap-gui_en.ts \
               resources/translations/tarsnap-gui_ro.ts

include(../tests-include.pro)

test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}"
