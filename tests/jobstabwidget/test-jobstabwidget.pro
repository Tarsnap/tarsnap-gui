TARGET = test-jobstabwidget
QT = core gui widgets sql network
TOPDIR = ../..

VALGRIND = true

FORMS +=								\
	$$TOPDIR/forms/archivelistwidgetitem.ui				\
	$$TOPDIR/forms/filepickerwidget.ui				\
	$$TOPDIR/forms/joblistwidgetitem.ui				\
	$$TOPDIR/forms/jobstabwidget.ui					\
	$$TOPDIR/forms/jobwidget.ui					\
	$$TOPDIR/forms/restoredialog.ui

# Needed for the database template and icons
RESOURCES += $$TOPDIR/resources/resources.qrc

HEADERS  +=								\
	$$TOPDIR/lib/core/TSettings.h					\
	$$TOPDIR/lib/widgets/TElidedLabel.h				\
	$$TOPDIR/src/backuptask.h					\
	$$TOPDIR/src/basetask.h						\
	$$TOPDIR/src/customfilesystemmodel.h				\
	$$TOPDIR/src/humanbytes.h					\
	$$TOPDIR/src/messages/archivefilestat.h				\
	$$TOPDIR/src/messages/archiveptr.h				\
	$$TOPDIR/src/messages/archiverestoreoptions.h			\
	$$TOPDIR/src/messages/backuptaskdataptr.h			\
	$$TOPDIR/src/messages/jobptr.h					\
	$$TOPDIR/src/parsearchivelistingtask.h				\
	$$TOPDIR/src/persistentmodel/archive.h				\
	$$TOPDIR/src/persistentmodel/job.h				\
	$$TOPDIR/src/persistentmodel/persistentobject.h			\
	$$TOPDIR/src/persistentmodel/persistentstore.h			\
	$$TOPDIR/src/persistentmodel/upgrade-store.h			\
	$$TOPDIR/src/tasks/tasks-utils.h				\
	$$TOPDIR/src/widgets/archivelistwidget.h			\
	$$TOPDIR/src/widgets/archivelistwidgetitem.h			\
	$$TOPDIR/src/widgets/elidedannotatedlabel.h			\
	$$TOPDIR/src/widgets/elidedclickablelabel.h			\
	$$TOPDIR/src/widgets/filepickerwidget.h				\
	$$TOPDIR/src/widgets/joblistwidget.h				\
	$$TOPDIR/src/widgets/joblistwidgetitem.h			\
	$$TOPDIR/src/widgets/jobstabwidget.h				\
	$$TOPDIR/src/widgets/jobwidget.h				\
	$$TOPDIR/src/widgets/restoredialog.h				\
	../qtest-platform.h

SOURCES += test-jobstabwidget.cpp					\
	$$TOPDIR/lib/core/TSettings.cpp					\
	$$TOPDIR/lib/widgets/TElidedLabel.cpp				\
	$$TOPDIR/src/backuptask.cpp					\
	$$TOPDIR/src/basetask.cpp					\
	$$TOPDIR/src/customfilesystemmodel.cpp				\
	$$TOPDIR/src/humanbytes.cpp					\
	$$TOPDIR/src/parsearchivelistingtask.cpp			\
	$$TOPDIR/src/persistentmodel/archive.cpp			\
	$$TOPDIR/src/persistentmodel/job.cpp				\
	$$TOPDIR/src/persistentmodel/persistentobject.cpp		\
	$$TOPDIR/src/persistentmodel/persistentstore.cpp		\
	$$TOPDIR/src/persistentmodel/upgrade-store.cpp			\
	$$TOPDIR/src/tasks/tasks-utils.cpp				\
	$$TOPDIR/src/widgets/archivelistwidget.cpp			\
	$$TOPDIR/src/widgets/archivelistwidgetitem.cpp			\
	$$TOPDIR/src/widgets/elidedannotatedlabel.cpp			\
	$$TOPDIR/src/widgets/elidedclickablelabel.cpp			\
	$$TOPDIR/src/widgets/filepickerwidget.cpp			\
	$$TOPDIR/src/widgets/joblistwidget.cpp				\
	$$TOPDIR/src/widgets/joblistwidgetitem.cpp			\
	$$TOPDIR/src/widgets/jobstabwidget.cpp				\
	$$TOPDIR/src/widgets/jobwidget.cpp				\
	$$TOPDIR/src/widgets/restoredialog.cpp

include(../tests-include.pri)

test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}";	\
	cp confdir/* "$${TEST_HOME}/$${TARGET}"
