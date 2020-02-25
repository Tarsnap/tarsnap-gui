TARGET = test-lib-widgets
QT = core gui widgets

VALGRIND = true

FORMS +=						\
	../../lib/forms/PathComboBrowse.ui		\
	../../lib/forms/TPathLineBrowse.ui		\
	../../lib/forms/TWizard.ui

RESOURCES += ../../resources/resources.qrc

HEADERS  +=						\
	../../lib/widgets/ElidedLabel.h			\
	../../lib/widgets/OkLabel.h			\
	../../lib/widgets/PathComboBrowse.h		\
	../../lib/widgets/TPathLineBrowse.h		\
	../../lib/widgets/TWizard.h			\
	../../lib/widgets/TWizardPage.h			\
	../../tests/qtest-platform.h

SOURCES += test-lib-widgets.cpp				\
	../../lib/core/TSettings.cpp			\
	../../lib/widgets/ElidedLabel.cpp		\
	../../lib/widgets/OkLabel.cpp			\
	../../lib/widgets/PathComboBrowse.cpp		\
	../../lib/widgets/TPathLineBrowse.cpp		\
	../../lib/widgets/TWizard.cpp			\
	../../lib/widgets/TWizardPage.cpp

include(../tests-include.pro)
