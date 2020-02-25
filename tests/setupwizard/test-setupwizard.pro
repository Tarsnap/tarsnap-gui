TARGET = test-setupwizard
QT = core gui widgets network

VALGRIND = true

FORMS +=						\
	../../forms/setupwizard.ui			\
	../../forms/setupwizard_cli.ui			\
	../../forms/setupwizard_final.ui		\
	../../forms/setupwizard_intro.ui		\
	../../forms/setupwizard_register.ui		\
	../../lib/forms/PathComboBrowse.ui		\
	../../lib/forms/TPathLineBrowse.ui		\
	../../lib/forms/TWizard.ui

HEADERS  +=						\
	compare-settings.h				\
	../../lib/core/TSettings.h			\
	../../lib/widgets/ElidedLabel.h			\
	../../lib/widgets/OkLabel.h			\
	../../lib/widgets/PathComboBrowse.h		\
	../../lib/widgets/TPathLineBrowse.h		\
	../../lib/widgets/TWizard.h			\
	../../lib/widgets/TWizardPage.h			\
	../../src/setupwizard/setupwizard.h		\
	../../src/setupwizard/setupwizard_cli.h		\
	../../src/setupwizard/setupwizard_final.h	\
	../../src/setupwizard/setupwizard_intro.h	\
	../../src/setupwizard/setupwizard_register.h	\
	../../src/tasks/tasks-utils.h			\
	../../src/utils.h				\
	../../src/widgets/busywidget.h			\
	../../tests/qtest-platform.h

SOURCES += test-setupwizard.cpp				\
	compare-settings.cpp				\
	../../lib/core/TSettings.cpp			\
	../../lib/widgets/ElidedLabel.cpp		\
	../../lib/widgets/OkLabel.cpp			\
	../../lib/widgets/PathComboBrowse.cpp		\
	../../lib/widgets/TPathLineBrowse.cpp		\
	../../lib/widgets/TWizard.cpp			\
	../../lib/widgets/TWizardPage.cpp		\
	../../src/setupwizard/setupwizard.cpp		\
	../../src/setupwizard/setupwizard_cli.cpp	\
	../../src/setupwizard/setupwizard_final.cpp	\
	../../src/setupwizard/setupwizard_intro.cpp	\
	../../src/setupwizard/setupwizard_register.cpp	\
	../../src/tasks/tasks-utils.cpp			\
	../../src/utils.cpp				\
	../../src/widgets/busywidget.cpp

RESOURCES += ../../resources/resources.qrc

include(../tests-include.pro)
