TARGET = test-helpwidget
QT = core gui widgets
TOPDIR = ../..

VALGRIND = true

FORMS +=								\
	$$TOPDIR/forms/aboutdialog.ui					\
	$$TOPDIR/forms/consolelogdialog.ui				\
	$$TOPDIR/forms/helpwidget.ui

RESOURCES += $$TOPDIR/resources/resources.qrc

HEADERS  +=								\
	$$TOPDIR/lib/widgets/TPopupPushButton.h				\
	$$TOPDIR/lib/widgets/TTextView.h				\
	$$TOPDIR/src/widgets/aboutdialog.h				\
	$$TOPDIR/src/widgets/consolelogdialog.h				\
	$$TOPDIR/src/widgets/helpwidget.h				\
	$$TOPDIR/tests/qtest-platform.h

SOURCES += test-helpwidget.cpp						\
	$$TOPDIR/lib/widgets/TPopupPushButton.cpp			\
	$$TOPDIR/lib/widgets/TTextView.cpp				\
	$$TOPDIR/src/widgets/aboutdialog.cpp				\
	$$TOPDIR/src/widgets/consolelogdialog.cpp			\
	$$TOPDIR/src/widgets/helpwidget.cpp

include(../tests-include.pri)
