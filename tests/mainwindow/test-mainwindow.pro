TARGET = test-mainwindow
QT = core gui widgets network sql
TOPDIR = ../..

VALGRIND = true

# For TarsnapAccount
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

FORMS +=								\
	$$TOPDIR/forms/aboutdialog.ui					\
	$$TOPDIR/forms/archivelistwidgetitem.ui				\
	$$TOPDIR/forms/archivestabwidget.ui				\
	$$TOPDIR/forms/archivewidget.ui					\
	$$TOPDIR/forms/backuplistwidgetitem.ui				\
	$$TOPDIR/forms/backuptabwidget.ui				\
	$$TOPDIR/forms/consolelogdialog.ui				\
	$$TOPDIR/forms/filepickerdialog.ui				\
	$$TOPDIR/forms/filepickerwidget.ui				\
	$$TOPDIR/forms/helpwidget.ui					\
	$$TOPDIR/forms/joblistwidgetitem.ui				\
	$$TOPDIR/forms/jobstabwidget.ui					\
	$$TOPDIR/forms/jobwidget.ui					\
	$$TOPDIR/forms/logindialog.ui					\
	$$TOPDIR/forms/mainwindow.ui					\
	$$TOPDIR/forms/restoredialog.ui					\
	$$TOPDIR/forms/settingswidget.ui				\
	$$TOPDIR/forms/statisticsdialog.ui				\
	$$TOPDIR/forms/statusbarwidget.ui				\
	$$TOPDIR/forms/stoptasksdialog.ui				\
	$$TOPDIR/lib/forms/TTabWidget.ui

RESOURCES +=								\
	$$TOPDIR/lib/resources/lib-resources.qrc			\
	$$TOPDIR/resources/resources.qrc

HEADERS  +=								\
	$$TOPDIR/lib/core/ConsoleLog.h					\
	$$TOPDIR/lib/core/TSettings.h					\
	$$TOPDIR/lib/widgets/TBusyLabel.h				\
	$$TOPDIR/lib/widgets/TElidedLabel.h				\
	$$TOPDIR/lib/widgets/TPopupPushButton.h				\
	$$TOPDIR/lib/widgets/TTabWidget.h				\
	$$TOPDIR/lib/widgets/TTextView.h				\
	$$TOPDIR/src/backuptask.h					\
	$$TOPDIR/src/basetask.h						\
	$$TOPDIR/src/customfilesystemmodel.h				\
	$$TOPDIR/src/dir-utils.h					\
	$$TOPDIR/src/dirinfotask.h					\
	$$TOPDIR/src/filetablemodel.h					\
	$$TOPDIR/src/humanbytes.h					\
	$$TOPDIR/src/messages/archivefilestat.h				\
	$$TOPDIR/src/messages/archiveptr.h				\
	$$TOPDIR/src/messages/archiverestoreoptions.h			\
	$$TOPDIR/src/messages/backuptaskdataptr.h			\
	$$TOPDIR/src/messages/jobptr.h					\
	$$TOPDIR/src/messages/notification_info.h			\
	$$TOPDIR/src/parsearchivelistingtask.h				\
	$$TOPDIR/src/persistentmodel/archive.h				\
	$$TOPDIR/src/persistentmodel/job.h				\
	$$TOPDIR/src/persistentmodel/persistentobject.h			\
	$$TOPDIR/src/persistentmodel/persistentstore.h			\
	$$TOPDIR/src/persistentmodel/upgrade-store.h			\
	$$TOPDIR/src/scheduling.h					\
	$$TOPDIR/src/tarsnapaccount.h					\
	$$TOPDIR/src/tasks/tasks-utils.h				\
	$$TOPDIR/src/translator.h					\
	$$TOPDIR/src/widgets/aboutdialog.h				\
	$$TOPDIR/src/widgets/archivelistwidget.h			\
	$$TOPDIR/src/widgets/archivelistwidgetitem.h			\
	$$TOPDIR/src/widgets/archivestabwidget.h			\
	$$TOPDIR/src/widgets/archivewidget.h				\
	$$TOPDIR/src/widgets/backuplistwidget.h				\
	$$TOPDIR/src/widgets/backuplistwidgetitem.h			\
	$$TOPDIR/src/widgets/backuptabwidget.h				\
	$$TOPDIR/src/widgets/confirmationdialog.h			\
	$$TOPDIR/src/widgets/consolelogdialog.h				\
	$$TOPDIR/src/widgets/elidedannotatedlabel.h			\
	$$TOPDIR/src/widgets/elidedclickablelabel.h			\
	$$TOPDIR/src/widgets/filepickerdialog.h				\
	$$TOPDIR/src/widgets/filepickerwidget.h				\
	$$TOPDIR/src/widgets/helpwidget.h				\
	$$TOPDIR/src/widgets/joblistwidget.h				\
	$$TOPDIR/src/widgets/joblistwidgetitem.h			\
	$$TOPDIR/src/widgets/jobstabwidget.h				\
	$$TOPDIR/src/widgets/jobwidget.h				\
	$$TOPDIR/src/widgets/mainwindow.h				\
	$$TOPDIR/src/widgets/restoredialog.h				\
	$$TOPDIR/src/widgets/schedulingwidgets.h			\
	$$TOPDIR/src/widgets/settingswidget.h				\
	$$TOPDIR/src/widgets/statisticsdialog.h				\
	$$TOPDIR/src/widgets/statusbarwidget.h				\
	$$TOPDIR/src/widgets/stoptasksdialog.h				\
	$$TOPDIR/src/widgets/tarsnapaccountdialog.h			\
	$$TOPDIR/tests/qtest-platform.h

SOURCES += test-mainwindow.cpp						\
	$$TOPDIR/lib/core/ConsoleLog.cpp				\
	$$TOPDIR/lib/core/TSettings.cpp					\
	$$TOPDIR/lib/widgets/TBusyLabel.cpp				\
	$$TOPDIR/lib/widgets/TElidedLabel.cpp				\
	$$TOPDIR/lib/widgets/TPopupPushButton.cpp			\
	$$TOPDIR/lib/widgets/TTabWidget.cpp				\
	$$TOPDIR/lib/widgets/TTextView.cpp				\
	$$TOPDIR/src/backuptask.cpp					\
	$$TOPDIR/src/basetask.cpp					\
	$$TOPDIR/src/customfilesystemmodel.cpp				\
	$$TOPDIR/src/dir-utils.cpp					\
	$$TOPDIR/src/dirinfotask.cpp					\
	$$TOPDIR/src/filetablemodel.cpp					\
	$$TOPDIR/src/humanbytes.cpp					\
	$$TOPDIR/src/parsearchivelistingtask.cpp			\
	$$TOPDIR/src/persistentmodel/archive.cpp			\
	$$TOPDIR/src/persistentmodel/job.cpp				\
	$$TOPDIR/src/persistentmodel/persistentobject.cpp		\
	$$TOPDIR/src/persistentmodel/persistentstore.cpp		\
	$$TOPDIR/src/persistentmodel/upgrade-store.cpp			\
	$$TOPDIR/src/scheduling.cpp					\
	$$TOPDIR/src/tarsnapaccount.cpp					\
	$$TOPDIR/src/tasks/tasks-utils.cpp				\
	$$TOPDIR/src/translator.cpp					\
	$$TOPDIR/src/widgets/aboutdialog.cpp				\
	$$TOPDIR/src/widgets/archivelistwidget.cpp			\
	$$TOPDIR/src/widgets/archivelistwidgetitem.cpp			\
	$$TOPDIR/src/widgets/archivestabwidget.cpp			\
	$$TOPDIR/src/widgets/archivewidget.cpp				\
	$$TOPDIR/src/widgets/backuplistwidget.cpp			\
	$$TOPDIR/src/widgets/backuplistwidgetitem.cpp			\
	$$TOPDIR/src/widgets/backuptabwidget.cpp			\
	$$TOPDIR/src/widgets/confirmationdialog.cpp			\
	$$TOPDIR/src/widgets/consolelogdialog.cpp			\
	$$TOPDIR/src/widgets/elidedannotatedlabel.cpp			\
	$$TOPDIR/src/widgets/elidedclickablelabel.cpp			\
	$$TOPDIR/src/widgets/filepickerdialog.cpp			\
	$$TOPDIR/src/widgets/filepickerwidget.cpp			\
	$$TOPDIR/src/widgets/helpwidget.cpp				\
	$$TOPDIR/src/widgets/joblistwidget.cpp				\
	$$TOPDIR/src/widgets/joblistwidgetitem.cpp			\
	$$TOPDIR/src/widgets/jobstabwidget.cpp				\
	$$TOPDIR/src/widgets/jobwidget.cpp				\
	$$TOPDIR/src/widgets/mainwindow.cpp				\
	$$TOPDIR/src/widgets/restoredialog.cpp				\
	$$TOPDIR/src/widgets/schedulingwidgets.cpp			\
	$$TOPDIR/src/widgets/settingswidget.cpp				\
	$$TOPDIR/src/widgets/statisticsdialog.cpp			\
	$$TOPDIR/src/widgets/statusbarwidget.cpp			\
	$$TOPDIR/src/widgets/stoptasksdialog.cpp			\
	$$TOPDIR/src/widgets/tarsnapaccountdialog.cpp

include(../tests-include.pri)

test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}";	\
	cp -r confdir/* "$${TEST_HOME}/$${TARGET}"
