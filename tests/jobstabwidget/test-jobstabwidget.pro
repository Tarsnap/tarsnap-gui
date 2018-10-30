TARGET = test-jobstabwidget
QT = core gui widgets sql network

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
	../../src/backuptask.h				\
	../../src/customfilesystemmodel.h		\
	../../src/persistentmodel/archive.h		\
	../../src/persistentmodel/job.h			\
	../../src/persistentmodel/persistentobject.h	\
	../../src/persistentmodel/persistentstore.h	\
	../../src/utils.h				\
	../../src/widgets/archivelistwidget.h		\
	../../src/widgets/archivelistwidgetitem.h	\
	../../src/widgets/elidedlabel.h			\
	../../src/widgets/filepickerwidget.h		\
	../../src/widgets/joblistwidget.h		\
	../../src/widgets/joblistwidgetitem.h		\
	../../src/widgets/jobstabwidget.h		\
	../../src/widgets/jobwidget.h			\
	../../src/widgets/restoredialog.h		\
	../qtest-platform.h

SOURCES += test-jobstabwidget.cpp			\
	../../lib/core/TSettings.cpp			\
	../../src/backuptask.cpp			\
	../../src/customfilesystemmodel.cpp		\
	../../src/persistentmodel/archive.cpp		\
	../../src/persistentmodel/job.cpp		\
	../../src/persistentmodel/persistentobject.cpp	\
	../../src/persistentmodel/persistentstore.cpp	\
	../../src/utils.cpp				\
	../../src/widgets/archivelistwidget.cpp		\
	../../src/widgets/archivelistwidgetitem.cpp	\
	../../src/widgets/elidedlabel.cpp		\
	../../src/widgets/filepickerwidget.cpp		\
	../../src/widgets/joblistwidget.cpp		\
	../../src/widgets/joblistwidgetitem.cpp		\
	../../src/widgets/jobstabwidget.cpp		\
	../../src/widgets/jobwidget.cpp			\
	../../src/widgets/restoredialog.cpp

include(../tests-include.pro)

test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}";	\
	cp confdir/* "$${TEST_HOME}/$${TARGET}"
