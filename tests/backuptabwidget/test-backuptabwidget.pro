TARGET = test-backuptabwidget
QT = core gui widgets sql

FORMS += ../../forms/backuptabwidget.ui

HEADERS  +=						\
	../../lib/core/TSettings.h			\
	../../src/backuptask.h				\
	../../src/customfilesystemmodel.h		\
	../../src/utils.h				\
	../../src/widgets/backuplistwidget.h		\
	../../src/widgets/backuplistwidgetitem.h	\
	../../src/widgets/backuptabwidget.h		\
	../../src/widgets/elidedlabel.h			\
	../../src/widgets/filepickerdialog.h		\
	../../src/widgets/filepickerwidget.h

SOURCES += test-backuptabwidget.cpp			\
	../../lib/core/TSettings.cpp			\
	../../src/backuptask.cpp			\
	../../src/customfilesystemmodel.cpp		\
	../../src/utils.cpp				\
	../../src/widgets/backuplistwidget.cpp		\
	../../src/widgets/backuplistwidgetitem.cpp	\
	../../src/widgets/backuptabwidget.cpp		\
	../../src/widgets/elidedlabel.cpp		\
	../../src/widgets/filepickerdialog.cpp		\
	../../src/widgets/filepickerwidget.cpp


include(../tests-include.pro)
