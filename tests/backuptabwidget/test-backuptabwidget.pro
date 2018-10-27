TARGET = test-backuptabwidget
QT = core gui widgets sql

FORMS += ../../forms/backuptabwidget.ui

HEADERS  +=						\
	../../lib/core/TSettings.h			\
	../../src/utils.h				\
	../../src/widgets/backuptabwidget.h		\
	../../src/widgets/elidedlabel.h

SOURCES += test-backuptabwidget.cpp			\
	../../lib/core/TSettings.cpp			\
	../../src/utils.cpp				\
	../../src/widgets/backuptabwidget.cpp		\
	../../src/widgets/elidedlabel.cpp

include(../tests-include.pro)
