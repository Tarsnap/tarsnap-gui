TARGET = test-backuptabwidget
QT = core gui widgets sql

FORMS += ../../forms/backuptabwidget.ui

HEADERS  +=						\
	../../src/widgets/backuptabwidget.h		\
	../../src/widgets/elidedlabel.h

SOURCES += test-backuptabwidget.cpp			\
	../../src/widgets/backuptabwidget.cpp		\
	../../src/widgets/elidedlabel.cpp

include(../tests-include.pro)
