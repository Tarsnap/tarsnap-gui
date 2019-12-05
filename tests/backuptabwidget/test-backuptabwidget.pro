TARGET = test-backuptabwidget
QT = core gui widgets sql

VALGRIND = true

FORMS += ../../forms/backuptabwidget.ui			\
	../../forms/backuplistwidgetitem.ui		\
	../../forms/filepickerdialog.ui			\
	../../forms/filepickerwidget.ui

HEADERS  +=						\
	../../lib/core/TSettings.h			\
	../../src/backuptask.h				\
	../../src/customfilesystemmodel.h		\
	../../src/tasks/tasks-utils.h			\
	../../src/utils.h				\
	../../src/widgets/backuplistwidget.h		\
	../../src/widgets/backuplistwidgetitem.h	\
	../../src/widgets/backuptabwidget.h		\
	../../src/widgets/elidedclickablelabel.h	\
	../../src/widgets/elidedlabel.h			\
	../../src/widgets/filepickerdialog.h		\
	../../src/widgets/filepickerwidget.h		\
	../qtest-platform.h

SOURCES += test-backuptabwidget.cpp			\
	../../lib/core/TSettings.cpp			\
	../../src/backuptask.cpp			\
	../../src/customfilesystemmodel.cpp		\
	../../src/tasks/tasks-utils.cpp			\
	../../src/utils.cpp				\
	../../src/widgets/backuplistwidget.cpp		\
	../../src/widgets/backuplistwidgetitem.cpp	\
	../../src/widgets/backuptabwidget.cpp		\
	../../src/widgets/elidedclickablelabel.cpp	\
	../../src/widgets/elidedlabel.cpp		\
	../../src/widgets/filepickerdialog.cpp		\
	../../src/widgets/filepickerwidget.cpp


include(../tests-include.pro)
