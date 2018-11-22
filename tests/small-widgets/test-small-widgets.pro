TARGET = test-small-widgets
QT = core gui widgets

HEADERS  +=						\
	../../lib/core/TSettings.h			\
	../../src/widgets/elidedlabel.h			\
	../../tests/qtest-platform.h

SOURCES += test-small-widgets.cpp			\
	../../src/widgets/elidedlabel.cpp

include(../tests-include.pro)
