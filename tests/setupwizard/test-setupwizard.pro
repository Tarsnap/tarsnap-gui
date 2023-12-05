TARGET = test-setupwizard
QT = core gui widgets network
TOPDIR = ../..

VALGRIND = true

FORMS +=								\
	$$TOPDIR/forms/setupwizard.ui					\
	$$TOPDIR/forms/setupwizard_cli.ui				\
	$$TOPDIR/forms/setupwizard_final.ui				\
	$$TOPDIR/forms/setupwizard_intro.ui				\
	$$TOPDIR/forms/setupwizard_register.ui				\
	$$TOPDIR/lib/forms/TPathComboBrowse.ui				\
	$$TOPDIR/lib/forms/TPathLineBrowse.ui				\
	$$TOPDIR/lib/forms/TProgressWidget.ui				\
	$$TOPDIR/lib/forms/TWizard.ui

HEADERS  +=								\
	compare-settings.h						\
	$$TOPDIR/lib/core/TSettings.h					\
	$$TOPDIR/lib/widgets/TAsideLabel.h				\
	$$TOPDIR/lib/widgets/TBusyLabel.h				\
	$$TOPDIR/lib/widgets/TElidedLabel.h				\
	$$TOPDIR/lib/widgets/TOkLabel.h					\
	$$TOPDIR/lib/widgets/TPathComboBrowse.h				\
	$$TOPDIR/lib/widgets/TPathLineBrowse.h				\
	$$TOPDIR/lib/widgets/TProgressWidget.h				\
	$$TOPDIR/lib/widgets/TWizard.h					\
	$$TOPDIR/lib/widgets/TWizardPage.h				\
	$$TOPDIR/src/dir-utils.h					\
	$$TOPDIR/src/setupwizard/setupwizard.h				\
	$$TOPDIR/src/setupwizard/setupwizard_cli.h			\
	$$TOPDIR/src/setupwizard/setupwizard_final.h			\
	$$TOPDIR/src/setupwizard/setupwizard_intro.h			\
	$$TOPDIR/src/setupwizard/setupwizard_register.h			\
	$$TOPDIR/src/tasks/tasks-utils.h				\
	$$TOPDIR/tests/qtest-platform.h

SOURCES += test-setupwizard.cpp						\
	compare-settings.cpp						\
	$$TOPDIR/lib/core/TSettings.cpp					\
	$$TOPDIR/lib/widgets/TAsideLabel.cpp				\
	$$TOPDIR/lib/widgets/TBusyLabel.cpp				\
	$$TOPDIR/lib/widgets/TElidedLabel.cpp				\
	$$TOPDIR/lib/widgets/TOkLabel.cpp				\
	$$TOPDIR/lib/widgets/TPathComboBrowse.cpp			\
	$$TOPDIR/lib/widgets/TPathLineBrowse.cpp			\
	$$TOPDIR/lib/widgets/TProgressWidget.cpp			\
	$$TOPDIR/lib/widgets/TWizard.cpp				\
	$$TOPDIR/lib/widgets/TWizardPage.cpp				\
	$$TOPDIR/src/dir-utils.cpp					\
	$$TOPDIR/src/setupwizard/setupwizard.cpp			\
	$$TOPDIR/src/setupwizard/setupwizard_cli.cpp			\
	$$TOPDIR/src/setupwizard/setupwizard_final.cpp			\
	$$TOPDIR/src/setupwizard/setupwizard_intro.cpp			\
	$$TOPDIR/src/setupwizard/setupwizard_register.cpp		\
	$$TOPDIR/src/tasks/tasks-utils.cpp

RESOURCES +=								\
	$$TOPDIR/lib/resources/lib-resources.qrc			\
	$$TOPDIR/resources/resources.qrc

include(../tests-include.pri)
