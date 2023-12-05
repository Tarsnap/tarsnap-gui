TARGET = test-lib-widgets
QT = core gui widgets
TOPDIR = ../..

VALGRIND = true

FORMS +=								\
	$$TOPDIR/lib/forms/TPathComboBrowse.ui				\
	$$TOPDIR/lib/forms/TPathLineBrowse.ui				\
	$$TOPDIR/lib/forms/TProgressWidget.ui				\
	$$TOPDIR/lib/forms/TTabWidget.ui				\
	$$TOPDIR/lib/forms/TWizard.ui

RESOURCES += $$TOPDIR/lib/resources/lib-resources.qrc			\
	$$TOPDIR/resources/resources.qrc

HEADERS  +=								\
	$$TOPDIR/lib/widgets/TAsideLabel.h				\
	$$TOPDIR/lib/widgets/TBusyLabel.h				\
	$$TOPDIR/lib/widgets/TElidedLabel.h				\
	$$TOPDIR/lib/widgets/TOkLabel.h					\
	$$TOPDIR/lib/widgets/TPathComboBrowse.h				\
	$$TOPDIR/lib/widgets/TPathLineBrowse.h				\
	$$TOPDIR/lib/widgets/TProgressWidget.h				\
	$$TOPDIR/lib/widgets/TTabWidget.h				\
	$$TOPDIR/lib/widgets/TTextView.h				\
	$$TOPDIR/lib/widgets/TWizard.h					\
	$$TOPDIR/lib/widgets/TWizardPage.h				\
	$$TOPDIR/tests/qtest-platform.h

SOURCES += test-lib-widgets.cpp						\
	$$TOPDIR/lib/core/TSettings.cpp					\
	$$TOPDIR/lib/widgets/TAsideLabel.cpp				\
	$$TOPDIR/lib/widgets/TBusyLabel.cpp				\
	$$TOPDIR/lib/widgets/TElidedLabel.cpp				\
	$$TOPDIR/lib/widgets/TOkLabel.cpp				\
	$$TOPDIR/lib/widgets/TPathComboBrowse.cpp			\
	$$TOPDIR/lib/widgets/TPathLineBrowse.cpp			\
	$$TOPDIR/lib/widgets/TProgressWidget.cpp			\
	$$TOPDIR/lib/widgets/TTabWidget.cpp				\
	$$TOPDIR/lib/widgets/TTextView.cpp				\
	$$TOPDIR/lib/widgets/TWizard.cpp				\
	$$TOPDIR/lib/widgets/TWizardPage.cpp

include(../tests-include.pri)
