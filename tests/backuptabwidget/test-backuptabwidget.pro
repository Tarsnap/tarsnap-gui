TARGET = test-backuptabwidget
QT = core gui widgets sql

VALGRIND = true

FORMS += ../../forms/backuptabwidget.ui			\
	../../forms/backuplistwidgetitem.ui		\
	../../forms/filepickerdialog.ui			\
	../../forms/filepickerwidget.ui

HEADERS  +=						\
	../../lib/core/TSettings.h			\
	../../lib/widgets/TElidedLabel.h		\
	../../src/backuptask.h				\
	../../src/basetask.h				\
	../../src/customfilesystemmodel.h		\
	../../src/dirinfotask.h				\
	../../src/humanbytes.h				\
	../../src/persistentmodel/archive.h		\
	../../src/persistentmodel/job.h			\
	../../src/persistentmodel/persistentobject.h	\
	../../src/persistentmodel/persistentstore.h	\
	../../src/persistentmodel/upgrade-store.h	\
	../../src/messages/archiveptr.h			\
	../../src/messages/backuptaskdataptr.h		\
	../../src/messages/jobptr.h			\
	../../src/tasks/tasks-utils.h			\
	../../src/widgets/backuplistwidget.h		\
	../../src/widgets/backuplistwidgetitem.h	\
	../../src/widgets/backuptabwidget.h		\
	../../src/widgets/elidedclickablelabel.h	\
	../../src/widgets/filepickerdialog.h		\
	../../src/widgets/filepickerwidget.h		\
	../qtest-platform.h

SOURCES += test-backuptabwidget.cpp			\
	../../lib/core/TSettings.cpp			\
	../../lib/widgets/TElidedLabel.cpp		\
	../../src/backuptask.cpp			\
	../../src/basetask.cpp				\
	../../src/customfilesystemmodel.cpp		\
	../../src/dirinfotask.cpp			\
	../../src/humanbytes.cpp			\
	../../src/tasks/tasks-utils.cpp			\
	../../src/persistentmodel/archive.cpp		\
	../../src/persistentmodel/job.cpp		\
	../../src/persistentmodel/persistentobject.cpp	\
	../../src/persistentmodel/persistentstore.cpp	\
	../../src/persistentmodel/upgrade-store.cpp	\
	../../src/widgets/backuplistwidget.cpp		\
	../../src/widgets/backuplistwidgetitem.cpp	\
	../../src/widgets/backuptabwidget.cpp		\
	../../src/widgets/elidedclickablelabel.cpp	\
	../../src/widgets/filepickerdialog.cpp		\
	../../src/widgets/filepickerwidget.cpp


include(../tests-include.pro)
