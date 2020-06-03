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
	../../lib/forms/TPathComboBrowse.ui		\
	../../lib/forms/TPathLineBrowse.ui		\
	../../lib/forms/TWizard.ui

SOURCES += test-app-setup.cpp				\
	../../lib/core/ConsoleLog.cpp			\
	../../lib/core/TSettings.cpp			\
	../../lib/util/optparse.c			\
	../../lib/util/optparse_helper.c		\
	../../lib/widgets/TBusyLabel.cpp		\
	../../lib/widgets/TElidedLabel.cpp		\
	../../lib/widgets/TOkLabel.cpp			\
	../../lib/widgets/TPathComboBrowse.cpp		\
	../../lib/widgets/TPathLineBrowse.cpp		\
	../../lib/widgets/TWizard.cpp			\
	../../lib/widgets/TWizardPage.cpp		\
	../../libcperciva/util/getopt.c			\
	../../libcperciva/util/warnp.c			\
	../../src/app-setup.cpp				\
	../../src/messages/archivefilestat.h		\
	../../src/backenddata.cpp			\
	../../src/backuptask.cpp			\
	../../src/basetask.cpp				\
	../../src/cmdlinetask.cpp			\
	../../src/dir-utils.cpp				\
	../../src/filetablemodel.cpp			\
	../../src/humanbytes.cpp			\
	../../src/init-shared.cpp			\
	../../src/jobrunner.cpp				\
	../../src/parsearchivelistingtask.cpp		\
	../../src/persistentmodel/archive.cpp		\
	../../src/persistentmodel/job.cpp		\
	../../src/persistentmodel/journal.cpp		\
	../../src/persistentmodel/persistentobject.cpp	\
	../../src/persistentmodel/persistentstore.cpp	\
	../../src/persistentmodel/upgrade-store.cpp	\
	../../src/scheduling.cpp			\
	../../src/setupwizard/setupwizard.cpp		\
	../../src/setupwizard/setupwizard_cli.cpp	\
	../../src/setupwizard/setupwizard_final.cpp	\
	../../src/setupwizard/setupwizard_intro.cpp	\
	../../src/setupwizard/setupwizard_register.cpp	\
	../../src/taskmanager.cpp			\
	../../src/taskqueuer.cpp			\
	../../src/tasks/tasks-misc.cpp			\
	../../src/tasks/tasks-setup.cpp			\
	../../src/tasks/tasks-tarsnap.cpp		\
	../../src/tasks/tasks-utils.cpp			\
	../../src/translator.cpp

HEADERS +=						\
	../../lib/core/ConsoleLog.h			\
	../../lib/core/LogEntry.h			\
	../../lib/core/TSettings.h			\
	../../lib/util/optparse.h			\
	../../lib/util/optparse_helper.h		\
	../../lib/widgets/TBusyLabel.h			\
	../../lib/widgets/TElidedLabel.h		\
	../../lib/widgets/TOkLabel.h			\
	../../lib/widgets/TPathComboBrowse.h		\
	../../lib/widgets/TPathLineBrowse.h		\
	../../lib/widgets/TWizard.h			\
	../../lib/widgets/TWizardPage.h			\
	../../libcperciva/util/getopt.h			\
	../../libcperciva/util/warnp.h			\
	../../src/app-setup.h				\
	../../src/backenddata.h				\
	../../src/backuptask.h				\
	../../src/basetask.h				\
	../../src/cmdlinetask.h				\
	../../src/dir-utils.h				\
	../../src/filetablemodel.h			\
	../../src/humanbytes.h				\
	../../src/init-shared.h				\
	../../src/jobrunner.h				\
	../../src/messages/archiveptr.h			\
	../../src/messages/archiverestoreoptions.h	\
	../../src/messages/backuptaskdataptr.h		\
	../../src/messages/jobptr.h			\
	../../src/messages/notification_info.h		\
	../../src/messages/tarsnaperror.h		\
	../../src/messages/taskstatus.h			\
	../../src/parsearchivelistingtask.h		\
	../../src/persistentmodel/archive.h		\
	../../src/persistentmodel/job.h			\
	../../src/persistentmodel/journal.h		\
	../../src/persistentmodel/persistentobject.h	\
	../../src/persistentmodel/persistentstore.h	\
	../../src/persistentmodel/upgrade-store.h	\
	../../src/scheduling.h				\
	../../src/setupwizard/setupwizard.h		\
	../../src/setupwizard/setupwizard_cli.h		\
	../../src/setupwizard/setupwizard_final.h	\
	../../src/setupwizard/setupwizard_intro.h	\
	../../src/setupwizard/setupwizard_register.h	\
	../../src/taskmanager.h				\
	../../src/taskqueuer.h				\
	../../src/tasks/tasks-defs.h			\
	../../src/tasks/tasks-misc.h			\
	../../src/tasks/tasks-setup.h			\
	../../src/tasks/tasks-tarsnap.h			\
	../../src/tasks/tasks-utils.h			\
	../../src/translator.h

RESOURCES +=						\
	../../lib/resources/lib-resources.qrc		\
 	../../resources/resources.qrc

TRANSLATIONS = resources/translations/tarsnap-gui_en.ts \
               resources/translations/tarsnap-gui_ro.ts

include(../tests-include.pro)

test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}"
