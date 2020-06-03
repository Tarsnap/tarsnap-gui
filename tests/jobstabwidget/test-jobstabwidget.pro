TARGET = test-jobstabwidget
QT = core gui widgets sql network

VALGRIND = true

FORMS +=							\
	../../forms/archivelistwidgetitem.ui			\
	../../forms/filepickerwidget.ui				\
	../../forms/joblistwidgetitem.ui			\
	../../forms/jobstabwidget.ui				\
	../../forms/jobwidget.ui				\
	../../forms/restoredialog.ui

# Needed for the database template and icons
RESOURCES += ../../resources/resources.qrc

HEADERS  +=						\
	../../lib/core/TSettings.h			\
	../../lib/widgets/TElidedLabel.h		\
	../../src/backuptask.h				\
	../../src/basetask.h				\
	../../src/customfilesystemmodel.h		\
	../../src/humanbytes.h				\
	../../src/messages/archivefilestat.h		\
	../../src/messages/archiveptr.h			\
	../../src/messages/archiverestoreoptions.h	\
	../../src/messages/backuptaskdataptr.h		\
	../../src/messages/jobptr.h			\
	../../src/parsearchivelistingtask.h		\
	../../src/persistentmodel/archive.h		\
	../../src/persistentmodel/job.h			\
	../../src/persistentmodel/persistentobject.h	\
	../../src/persistentmodel/persistentstore.h	\
	../../src/persistentmodel/upgrade-store.h	\
	../../src/tasks/tasks-utils.h			\
	../../src/widgets/archivelistwidget.h		\
	../../src/widgets/archivelistwidgetitem.h	\
	../../src/widgets/elidedannotatedlabel.h	\
	../../src/widgets/elidedclickablelabel.h	\
	../../src/widgets/filepickerwidget.h		\
	../../src/widgets/joblistwidget.h		\
	../../src/widgets/joblistwidgetitem.h		\
	../../src/widgets/jobstabwidget.h		\
	../../src/widgets/jobwidget.h			\
	../../src/widgets/restoredialog.h		\
	../qtest-platform.h

SOURCES += test-jobstabwidget.cpp			\
	../../lib/core/TSettings.cpp			\
	../../lib/widgets/TElidedLabel.cpp		\
	../../src/backuptask.cpp			\
	../../src/basetask.cpp				\
	../../src/customfilesystemmodel.cpp		\
	../../src/humanbytes.cpp			\
	../../src/parsearchivelistingtask.cpp		\
	../../src/persistentmodel/archive.cpp		\
	../../src/persistentmodel/job.cpp		\
	../../src/persistentmodel/persistentobject.cpp	\
	../../src/persistentmodel/persistentstore.cpp	\
	../../src/persistentmodel/upgrade-store.cpp	\
	../../src/tasks/tasks-utils.cpp			\
	../../src/widgets/archivelistwidget.cpp		\
	../../src/widgets/archivelistwidgetitem.cpp	\
	../../src/widgets/elidedannotatedlabel.cpp	\
	../../src/widgets/elidedclickablelabel.cpp	\
	../../src/widgets/filepickerwidget.cpp		\
	../../src/widgets/joblistwidget.cpp		\
	../../src/widgets/joblistwidgetitem.cpp		\
	../../src/widgets/jobstabwidget.cpp		\
	../../src/widgets/jobwidget.cpp			\
	../../src/widgets/restoredialog.cpp

include(../tests-include.pro)

test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}";	\
	cp confdir/* "$${TEST_HOME}/$${TARGET}"
