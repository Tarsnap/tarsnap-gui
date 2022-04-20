TARGET = test-helpwidget
QT = core gui widgets

VALGRIND = true

FORMS +=						\
	../../forms/aboutdialog.ui			\
	../../forms/consolelogdialog.ui			\
	../../forms/helpwidget.ui

RESOURCES += ../../resources/resources.qrc

HEADERS  +=						\
	../../lib/widgets/TPopupPushButton.h		\
	../../lib/widgets/TTextView.h			\
	../../src/widgets/aboutdialog.h			\
	../../src/widgets/helpwidget.h			\
	../../tests/qtest-platform.h

SOURCES += test-helpwidget.cpp				\
	../../lib/widgets/TPopupPushButton.cpp		\
	../../lib/widgets/TTextView.cpp			\
	../../src/widgets/aboutdialog.cpp		\
	../../src/widgets/helpwidget.cpp

include(../tests-include.pri)
