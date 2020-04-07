TARGET = test-archivestabwidget
QT = core gui widgets sql

VALGRIND = true

FORMS +=						\
	../../forms/archivelistwidgetitem.ui		\
	../../forms/archivestabwidget.ui		\
	../../forms/archivewidget.ui			\
	../../forms/restoredialog.ui

RESOURCES += ../../resources/resources.qrc

HEADERS  +=						\
	../../lib/core/TSettings.h			\
	../../lib/widgets/TElidedLabel.h		\
	../../src/archivefilestat.h			\
	../../src/filetablemodel.h			\
	../../src/parsearchivelistingtask.h		\
	../../src/persistentmodel/archive.h		\
	../../src/persistentmodel/persistentobject.h	\
	../../src/persistentmodel/persistentstore.h	\
	../../src/tasks/tasks-utils.h			\
	../../src/utils.h				\
	../../src/widgets/archivelistwidget.h		\
	../../src/widgets/archivelistwidgetitem.h	\
	../../src/widgets/archivestabwidget.h		\
	../../src/widgets/archivewidget.h		\
	../../src/widgets/elidedannotatedlabel.h	\
	../../src/widgets/elidedclickablelabel.h	\
	../../src/widgets/restoredialog.h		\
	../qtest-platform.h

SOURCES += test-archivestabwidget.cpp			\
	../../lib/core/TSettings.cpp			\
	../../lib/widgets/TElidedLabel.cpp		\
	../../src/filetablemodel.cpp			\
	../../src/parsearchivelistingtask.cpp		\
	../../src/persistentmodel/archive.cpp		\
	../../src/persistentmodel/persistentobject.cpp	\
	../../src/persistentmodel/persistentstore.cpp	\
	../../src/tasks/tasks-utils.cpp			\
	../../src/utils.cpp				\
	../../src/widgets/archivelistwidget.cpp		\
	../../src/widgets/archivelistwidgetitem.cpp	\
	../../src/widgets/archivestabwidget.cpp		\
	../../src/widgets/archivewidget.cpp		\
	../../src/widgets/elidedannotatedlabel.cpp	\
	../../src/widgets/elidedclickablelabel.cpp	\
	../../src/widgets/restoredialog.cpp


include(../tests-include.pro)
