TARGET = test-helpwidget
QT = core gui widgets

VALGRIND = true

FORMS +=						\
	../../forms/aboutwidget.ui			\
	../../forms/consolewidget.ui			\
	../../forms/helpwidget.ui

RESOURCES += ../../resources/resources.qrc

HEADERS  +=						\
	../../lib/widgets/TTextView.h			\
	../../src/widgets/helpwidget.h			\
	../../src/widgets/popuppushbutton.h		\
	../../tests/qtest-platform.h

SOURCES += test-helpwidget.cpp				\
	../../lib/widgets/TTextView.cpp			\
	../../src/widgets/helpwidget.cpp		\
	../../src/widgets/popuppushbutton.cpp

include(../tests-include.pro)
