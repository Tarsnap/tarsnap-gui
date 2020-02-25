TARGET = test-lib-widgets
QT = core gui widgets

VALGRIND = true

FORMS +=						\
	../../lib/forms/TPathComboBrowse.ui		\
	../../lib/forms/TPathLineBrowse.ui		\
	../../lib/forms/TWizard.ui

RESOURCES += ../../resources/resources.qrc

HEADERS  +=						\
	../../lib/widgets/TElidedLabel.h		\
	../../lib/widgets/TOkLabel.h			\
	../../lib/widgets/TPathComboBrowse.h		\
	../../lib/widgets/TPathLineBrowse.h		\
	../../lib/widgets/TWizard.h			\
	../../lib/widgets/TWizardPage.h			\
	../../tests/qtest-platform.h

SOURCES += test-lib-widgets.cpp				\
	../../lib/core/TSettings.cpp			\
	../../lib/widgets/TElidedLabel.cpp		\
	../../lib/widgets/TOkLabel.cpp			\
	../../lib/widgets/TPathComboBrowse.cpp		\
	../../lib/widgets/TPathLineBrowse.cpp		\
	../../lib/widgets/TWizard.cpp			\
	../../lib/widgets/TWizardPage.cpp

include(../tests-include.pro)
