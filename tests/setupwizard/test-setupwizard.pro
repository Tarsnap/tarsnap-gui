TARGET = test-setupwizard
QT = core gui widgets network

VALGRIND = true

FORMS +=						\
	../../forms/setupwizard.ui			\
	../../forms/setupwizard_cli.ui			\
	../../forms/setupwizard_final.ui		\
	../../forms/setupwizard_intro.ui		\
	../../forms/setupwizard_register.ui		\
	../../lib/forms/TPathComboBrowse.ui		\
	../../lib/forms/TPathLineBrowse.ui		\
	../../lib/forms/TWizard.ui

HEADERS  +=						\
	compare-settings.h				\
	../../lib/core/TSettings.h			\
	../../lib/widgets/TBusyLabel.h			\
	../../lib/widgets/TElidedLabel.h		\
	../../lib/widgets/TOkLabel.h			\
	../../lib/widgets/TPathComboBrowse.h		\
	../../lib/widgets/TPathLineBrowse.h		\
	../../lib/widgets/TWizard.h			\
	../../lib/widgets/TWizardPage.h			\
	../../src/dir-utils.h				\
	../../src/setupwizard/setupwizard.h		\
	../../src/setupwizard/setupwizard_cli.h		\
	../../src/setupwizard/setupwizard_final.h	\
	../../src/setupwizard/setupwizard_intro.h	\
	../../src/setupwizard/setupwizard_register.h	\
	../../src/tasks/tasks-utils.h			\
	../../tests/qtest-platform.h

SOURCES += test-setupwizard.cpp				\
	compare-settings.cpp				\
	../../lib/core/TSettings.cpp			\
	../../lib/widgets/TBusyLabel.cpp		\
	../../lib/widgets/TElidedLabel.cpp		\
	../../lib/widgets/TOkLabel.cpp			\
	../../lib/widgets/TPathComboBrowse.cpp		\
	../../lib/widgets/TPathLineBrowse.cpp		\
	../../lib/widgets/TWizard.cpp			\
	../../lib/widgets/TWizardPage.cpp		\
	../../src/dir-utils.cpp				\
	../../src/setupwizard/setupwizard.cpp		\
	../../src/setupwizard/setupwizard_cli.cpp	\
	../../src/setupwizard/setupwizard_final.cpp	\
	../../src/setupwizard/setupwizard_intro.cpp	\
	../../src/setupwizard/setupwizard_register.cpp	\
	../../src/tasks/tasks-utils.cpp

RESOURCES +=						\
	../../lib/resources/lib-resources.qrc		\
	../../resources/resources.qrc

include(../tests-include.pro)
