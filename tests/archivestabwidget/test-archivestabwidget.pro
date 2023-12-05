TARGET = test-archivestabwidget
QT = core gui widgets sql
TOPDIR = ../..

VALGRIND = true

FORMS +=								\
	$$TOPDIR/forms/archivelistwidgetitem.ui				\
	$$TOPDIR/forms/archivestabwidget.ui				\
	$$TOPDIR/forms/archivewidget.ui					\
	$$TOPDIR/forms/restoredialog.ui

RESOURCES += $$TOPDIR/resources/resources.qrc

HEADERS  +=								\
	$$TOPDIR/lib/core/TSettings.h					\
	$$TOPDIR/lib/widgets/TElidedLabel.h				\
	$$TOPDIR/src/basetask.h						\
	$$TOPDIR/src/filetablemodel.h					\
	$$TOPDIR/src/humanbytes.h					\
	$$TOPDIR/src/messages/archivefilestat.h				\
	$$TOPDIR/src/messages/archiveptr.h				\
	$$TOPDIR/src/messages/archiverestoreoptions.h			\
	$$TOPDIR/src/parsearchivelistingtask.h				\
	$$TOPDIR/src/persistentmodel/archive.h				\
	$$TOPDIR/src/persistentmodel/persistentobject.h			\
	$$TOPDIR/src/persistentmodel/persistentstore.h			\
	$$TOPDIR/src/persistentmodel/upgrade-store.h			\
	$$TOPDIR/src/tasks/tasks-utils.h				\
	$$TOPDIR/src/widgets/archivelistwidget.h			\
	$$TOPDIR/src/widgets/archivelistwidgetitem.h			\
	$$TOPDIR/src/widgets/archivestabwidget.h			\
	$$TOPDIR/src/widgets/archivewidget.h				\
	$$TOPDIR/src/widgets/elidedannotatedlabel.h			\
	$$TOPDIR/src/widgets/elidedclickablelabel.h			\
	$$TOPDIR/src/widgets/restoredialog.h				\
	../qtest-platform.h

SOURCES += test-archivestabwidget.cpp					\
	$$TOPDIR/lib/core/TSettings.cpp					\
	$$TOPDIR/lib/widgets/TElidedLabel.cpp				\
	$$TOPDIR/src/basetask.cpp					\
	$$TOPDIR/src/filetablemodel.cpp					\
	$$TOPDIR/src/humanbytes.cpp					\
	$$TOPDIR/src/parsearchivelistingtask.cpp			\
	$$TOPDIR/src/persistentmodel/archive.cpp			\
	$$TOPDIR/src/persistentmodel/persistentobject.cpp		\
	$$TOPDIR/src/persistentmodel/persistentstore.cpp		\
	$$TOPDIR/src/persistentmodel/upgrade-store.cpp			\
	$$TOPDIR/src/tasks/tasks-utils.cpp				\
	$$TOPDIR/src/widgets/archivelistwidget.cpp			\
	$$TOPDIR/src/widgets/archivelistwidgetitem.cpp			\
	$$TOPDIR/src/widgets/archivestabwidget.cpp			\
	$$TOPDIR/src/widgets/archivewidget.cpp				\
	$$TOPDIR/src/widgets/elidedannotatedlabel.cpp			\
	$$TOPDIR/src/widgets/elidedclickablelabel.cpp			\
	$$TOPDIR/src/widgets/restoredialog.cpp


include(../tests-include.pri)
