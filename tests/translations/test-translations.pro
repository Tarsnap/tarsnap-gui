TARGET = test-translations
QT = core gui widgets

VALGRIND = true

FORMS +=						\
	../../lib/forms/TPathComboBrowse.ui

HEADERS  +=						\
	../../lib/core/TSettings.h			\
	../../lib/widgets/TElidedLabel.h		\
	../../lib/widgets/TOkLabel.h			\
	../../lib/widgets/TPathComboBrowse.h		\
	../../src/translator.cpp			\
	../../tests/qtest-platform.h

SOURCES += test-translations.cpp			\
	../../lib/core/TSettings.cpp			\
	../../lib/widgets/TElidedLabel.cpp		\
	../../lib/widgets/TOkLabel.cpp			\
	../../lib/widgets/TPathComboBrowse.cpp		\
	../../src/translator.cpp

RESOURCES +=						\
	../../lib/resources/lib-resources.qrc		\
	../../resources/resources.qrc

include(../tests-include.pro)

test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}";	\
	touch "$${TEST_HOME}/$${TARGET}/empty-file"
