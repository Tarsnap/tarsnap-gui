TARGET = test-backuptabwidget
QT = core gui widgets sql
TOPDIR = ../..

VALGRIND = true

FORMS += $$TOPDIR/forms/backuptabwidget.ui				\
	$$TOPDIR/forms/backuplistwidgetitem.ui				\
	$$TOPDIR/forms/filepickerdialog.ui				\
	$$TOPDIR/forms/filepickerwidget.ui

HEADERS  +=								\
	$$TOPDIR/lib/core/TSettings.h					\
	$$TOPDIR/lib/widgets/TElidedLabel.h				\
	$$TOPDIR/src/backuptask.h					\
	$$TOPDIR/src/basetask.h						\
	$$TOPDIR/src/customfilesystemmodel.h				\
	$$TOPDIR/src/dirinfotask.h					\
	$$TOPDIR/src/humanbytes.h					\
	$$TOPDIR/src/persistentmodel/archive.h				\
	$$TOPDIR/src/persistentmodel/job.h				\
	$$TOPDIR/src/persistentmodel/persistentobject.h			\
	$$TOPDIR/src/persistentmodel/persistentstore.h			\
	$$TOPDIR/src/persistentmodel/upgrade-store.h			\
	$$TOPDIR/src/messages/archiveptr.h				\
	$$TOPDIR/src/messages/backuptaskdataptr.h			\
	$$TOPDIR/src/messages/jobptr.h					\
	$$TOPDIR/src/tasks/tasks-utils.h				\
	$$TOPDIR/src/widgets/backuplistwidget.h				\
	$$TOPDIR/src/widgets/backuplistwidgetitem.h			\
	$$TOPDIR/src/widgets/backuptabwidget.h				\
	$$TOPDIR/src/widgets/elidedclickablelabel.h			\
	$$TOPDIR/src/widgets/filepickerdialog.h				\
	$$TOPDIR/src/widgets/filepickerwidget.h				\
	../qtest-platform.h

SOURCES += test-backuptabwidget.cpp					\
	$$TOPDIR/lib/core/TSettings.cpp					\
	$$TOPDIR/lib/widgets/TElidedLabel.cpp				\
	$$TOPDIR/src/backuptask.cpp					\
	$$TOPDIR/src/basetask.cpp					\
	$$TOPDIR/src/customfilesystemmodel.cpp				\
	$$TOPDIR/src/dirinfotask.cpp					\
	$$TOPDIR/src/humanbytes.cpp					\
	$$TOPDIR/src/tasks/tasks-utils.cpp				\
	$$TOPDIR/src/persistentmodel/archive.cpp			\
	$$TOPDIR/src/persistentmodel/job.cpp				\
	$$TOPDIR/src/persistentmodel/persistentobject.cpp		\
	$$TOPDIR/src/persistentmodel/persistentstore.cpp		\
	$$TOPDIR/src/persistentmodel/upgrade-store.cpp			\
	$$TOPDIR/src/widgets/backuplistwidget.cpp			\
	$$TOPDIR/src/widgets/backuplistwidgetitem.cpp			\
	$$TOPDIR/src/widgets/backuptabwidget.cpp			\
	$$TOPDIR/src/widgets/elidedclickablelabel.cpp			\
	$$TOPDIR/src/widgets/filepickerdialog.cpp			\
	$$TOPDIR/src/widgets/filepickerwidget.cpp


include(../tests-include.pri)
