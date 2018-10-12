TARGET = test-backuptabwidget
QT = core gui widgets sql

FORMS += ../../forms/backuptabwidget.ui

HEADERS  +=						\
	../../src/widgets/backuptabwidget.h

SOURCES += test-backuptabwidget.cpp			\
	../../src/widgets/backuptabwidget.cpp

include(../tests-include.pro)
