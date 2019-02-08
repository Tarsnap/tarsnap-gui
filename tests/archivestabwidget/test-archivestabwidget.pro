TARGET = test-archivestabwidget
QT = core gui widgets sql

VALGRIND = true

FORMS += ../../forms/archivestabwidget.ui		\
	../../forms/archivelistwidgetitem.ui

RESOURCES += ../../resources/resources.qrc

HEADERS  +=						\
	../../lib/core/TSettings.h			\
	../../src/filetablemodel.h			\
	../../src/persistentmodel/archive.h		\
	../../src/persistentmodel/persistentobject.h	\
	../../src/persistentmodel/persistentstore.h	\
	../../src/utils.h				\
	../../src/widgets/archivelistwidget.h		\
	../../src/widgets/archivelistwidgetitem.h	\
	../../src/widgets/archivestabwidget.h		\
	../../src/widgets/archivewidget.h		\
	../../src/widgets/elidedannotatedlabel.h		\
	../../src/widgets/elidedlabel.h			\
	../../src/widgets/restoredialog.h		\
	../qtest-platform.h

SOURCES += test-archivestabwidget.cpp			\
	../../lib/core/TSettings.cpp			\
	../../src/filetablemodel.cpp			\
	../../src/persistentmodel/archive.cpp		\
	../../src/persistentmodel/persistentobject.cpp	\
	../../src/persistentmodel/persistentstore.cpp	\
	../../src/utils.cpp				\
	../../src/widgets/archivelistwidget.cpp		\
	../../src/widgets/archivelistwidgetitem.cpp	\
	../../src/widgets/archivestabwidget.cpp		\
	../../src/widgets/archivewidget.cpp		\
	../../src/widgets/elidedannotatedlabel.cpp		\
	../../src/widgets/elidedlabel.cpp		\
	../../src/widgets/restoredialog.cpp


include(../tests-include.pro)
