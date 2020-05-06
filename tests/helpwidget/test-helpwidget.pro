TARGET = test-helpwidget
QT = core gui widgets

VALGRIND = true

FORMS +=						\
	../../forms/aboutwidget.ui			\
	../../forms/consolewidget.ui			\
	../../forms/helpwidget.ui

RESOURCES += ../../resources/resources.qrc

HEADERS  +=						\
	../../lib/widgets/TPopupPushButton.h		\
	../../lib/widgets/TTextView.h			\
	../../src/widgets/helpwidget.h			\
	../../tests/qtest-platform.h

SOURCES += test-helpwidget.cpp				\
	../../lib/widgets/TPopupPushButton.cpp		\
	../../lib/widgets/TTextView.cpp			\
	../../src/widgets/helpwidget.cpp

include(../tests-include.pro)
