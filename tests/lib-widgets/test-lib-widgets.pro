TARGET = test-lib-widgets
QT = core gui widgets

VALGRIND = true

FORMS +=						\
	../../lib/forms/PathLineBrowse.ui		\
	../../lib/forms/TWizard.ui

RESOURCES += ../../resources/resources.qrc

HEADERS  +=						\
	../../lib/widgets/ElidedLabel.h			\
	../../lib/widgets/PathLineBrowse.h		\
	../../lib/widgets/TWizard.h			\
	../../lib/widgets/TWizardPage.h			\
	../../tests/qtest-platform.h

SOURCES += test-lib-widgets.cpp				\
	../../lib/core/TSettings.cpp			\
	../../lib/widgets/ElidedLabel.cpp		\
	../../lib/widgets/PathLineBrowse.cpp		\
	../../lib/widgets/TWizard.cpp			\
	../../lib/widgets/TWizardPage.cpp

include(../tests-include.pro)
