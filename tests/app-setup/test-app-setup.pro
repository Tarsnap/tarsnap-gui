TARGET = test-app-setup
QT = core gui widgets network sql
TOPDIR = ../..

VALGRIND = true

# Used in lib/core/optparse.h
VERSION = test
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

FORMS +=								\
	$$TOPDIR/forms/setupwizard_cli.ui				\
	$$TOPDIR/forms/setupwizard_final.ui				\
	$$TOPDIR/forms/setupwizard_intro.ui				\
	$$TOPDIR/forms/setupwizard_register.ui				\
	$$TOPDIR/lib/forms/TPathComboBrowse.ui				\
	$$TOPDIR/lib/forms/TPathLineBrowse.ui				\
	$$TOPDIR/lib/forms/TProgressWidget.ui				\
	$$TOPDIR/lib/forms/TWizard.ui

SOURCES += test-app-setup.cpp						\
	$$TOPDIR/lib/core/ConsoleLog.cpp				\
	$$TOPDIR/lib/core/TSettings.cpp					\
	$$TOPDIR/lib/util/optparse.c					\
	$$TOPDIR/lib/util/optparse_helper.c				\
	$$TOPDIR/lib/widgets/TAsideLabel.cpp				\
	$$TOPDIR/lib/widgets/TBusyLabel.cpp				\
	$$TOPDIR/lib/widgets/TElidedLabel.cpp				\
	$$TOPDIR/lib/widgets/TOkLabel.cpp				\
	$$TOPDIR/lib/widgets/TPathComboBrowse.cpp			\
	$$TOPDIR/lib/widgets/TPathLineBrowse.cpp			\
	$$TOPDIR/lib/widgets/TProgressWidget.cpp			\
	$$TOPDIR/lib/widgets/TWizard.cpp				\
	$$TOPDIR/lib/widgets/TWizardPage.cpp				\
	$$TOPDIR/libcperciva/util/getopt.c				\
	$$TOPDIR/libcperciva/util/warnp.c				\
	$$TOPDIR/src/app-setup.cpp					\
	$$TOPDIR/src/messages/archivefilestat.h				\
	$$TOPDIR/src/backenddata.cpp					\
	$$TOPDIR/src/backuptask.cpp					\
	$$TOPDIR/src/basetask.cpp					\
	$$TOPDIR/src/cmdlinetask.cpp					\
	$$TOPDIR/src/dir-utils.cpp					\
	$$TOPDIR/src/filetablemodel.cpp					\
	$$TOPDIR/src/humanbytes.cpp					\
	$$TOPDIR/src/init-shared.cpp					\
	$$TOPDIR/src/jobrunner.cpp					\
	$$TOPDIR/src/parsearchivelistingtask.cpp			\
	$$TOPDIR/src/persistentmodel/archive.cpp			\
	$$TOPDIR/src/persistentmodel/job.cpp				\
	$$TOPDIR/src/persistentmodel/journal.cpp			\
	$$TOPDIR/src/persistentmodel/persistentobject.cpp		\
	$$TOPDIR/src/persistentmodel/persistentstore.cpp		\
	$$TOPDIR/src/persistentmodel/upgrade-store.cpp			\
	$$TOPDIR/src/scheduling.cpp					\
	$$TOPDIR/src/setupwizard/setupwizard.cpp			\
	$$TOPDIR/src/setupwizard/setupwizard_cli.cpp			\
	$$TOPDIR/src/setupwizard/setupwizard_final.cpp			\
	$$TOPDIR/src/setupwizard/setupwizard_intro.cpp			\
	$$TOPDIR/src/setupwizard/setupwizard_register.cpp		\
	$$TOPDIR/src/taskmanager.cpp					\
	$$TOPDIR/src/taskqueuer.cpp					\
	$$TOPDIR/src/tasks/tasks-misc.cpp				\
	$$TOPDIR/src/tasks/tasks-setup.cpp				\
	$$TOPDIR/src/tasks/tasks-tarsnap.cpp				\
	$$TOPDIR/src/tasks/tasks-utils.cpp				\
	$$TOPDIR/src/translator.cpp

HEADERS +=								\
	$$TOPDIR/lib/core/ConsoleLog.h					\
	$$TOPDIR/lib/core/LogEntry.h					\
	$$TOPDIR/lib/core/TSettings.h					\
	$$TOPDIR/lib/util/optparse.h					\
	$$TOPDIR/lib/util/optparse_helper.h				\
	$$TOPDIR/lib/widgets/TAsideLabel.h				\
	$$TOPDIR/lib/widgets/TBusyLabel.h				\
	$$TOPDIR/lib/widgets/TElidedLabel.h				\
	$$TOPDIR/lib/widgets/TOkLabel.h					\
	$$TOPDIR/lib/widgets/TPathComboBrowse.h				\
	$$TOPDIR/lib/widgets/TPathLineBrowse.h				\
	$$TOPDIR/lib/widgets/TProgressWidget.h				\
	$$TOPDIR/lib/widgets/TWizard.h					\
	$$TOPDIR/lib/widgets/TWizardPage.h				\
	$$TOPDIR/libcperciva/util/getopt.h				\
	$$TOPDIR/libcperciva/util/warnp.h				\
	$$TOPDIR/src/app-setup.h					\
	$$TOPDIR/src/backenddata.h					\
	$$TOPDIR/src/backuptask.h					\
	$$TOPDIR/src/basetask.h						\
	$$TOPDIR/src/cmdlinetask.h					\
	$$TOPDIR/src/dir-utils.h					\
	$$TOPDIR/src/filetablemodel.h					\
	$$TOPDIR/src/humanbytes.h					\
	$$TOPDIR/src/init-shared.h					\
	$$TOPDIR/src/jobrunner.h					\
	$$TOPDIR/src/messages/archiveptr.h				\
	$$TOPDIR/src/messages/archiverestoreoptions.h			\
	$$TOPDIR/src/messages/backuptaskdataptr.h			\
	$$TOPDIR/src/messages/jobptr.h					\
	$$TOPDIR/src/messages/notification_info.h			\
	$$TOPDIR/src/messages/tarsnaperror.h				\
	$$TOPDIR/src/messages/taskstatus.h				\
	$$TOPDIR/src/parsearchivelistingtask.h				\
	$$TOPDIR/src/persistentmodel/archive.h				\
	$$TOPDIR/src/persistentmodel/job.h				\
	$$TOPDIR/src/persistentmodel/journal.h				\
	$$TOPDIR/src/persistentmodel/persistentobject.h			\
	$$TOPDIR/src/persistentmodel/persistentstore.h			\
	$$TOPDIR/src/persistentmodel/upgrade-store.h			\
	$$TOPDIR/src/scheduling.h					\
	$$TOPDIR/src/setupwizard/setupwizard.h				\
	$$TOPDIR/src/setupwizard/setupwizard_cli.h			\
	$$TOPDIR/src/setupwizard/setupwizard_final.h			\
	$$TOPDIR/src/setupwizard/setupwizard_intro.h			\
	$$TOPDIR/src/setupwizard/setupwizard_register.h			\
	$$TOPDIR/src/taskmanager.h					\
	$$TOPDIR/src/taskqueuer.h					\
	$$TOPDIR/src/tasks/tasks-defs.h					\
	$$TOPDIR/src/tasks/tasks-misc.h					\
	$$TOPDIR/src/tasks/tasks-setup.h				\
	$$TOPDIR/src/tasks/tasks-tarsnap.h				\
	$$TOPDIR/src/tasks/tasks-utils.h				\
	$$TOPDIR/src/translator.h

RESOURCES +=								\
	$$TOPDIR/lib/resources/lib-resources.qrc			\
 	$$TOPDIR/resources/resources.qrc

TRANSLATIONS = resources/translations/tarsnap-gui_en.ts			\
               resources/translations/tarsnap-gui_ro.ts

include(../tests-include.pri)

test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}"
