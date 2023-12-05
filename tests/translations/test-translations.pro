TARGET = test-translations
QT = core gui widgets
TOPDIR = ../..

VALGRIND = true

FORMS +=								\
	$$TOPDIR/lib/forms/TPathComboBrowse.ui

HEADERS  +=								\
	$$TOPDIR/lib/core/TSettings.h					\
	$$TOPDIR/lib/widgets/TElidedLabel.h				\
	$$TOPDIR/lib/widgets/TOkLabel.h					\
	$$TOPDIR/lib/widgets/TPathComboBrowse.h				\
	$$TOPDIR/src/translator.cpp					\
	$$TOPDIR/src/widgets/confirmationdialog.h			\
	$$TOPDIR/tests/qtest-platform.h

SOURCES += test-translations.cpp					\
	$$TOPDIR/lib/core/TSettings.cpp					\
	$$TOPDIR/lib/widgets/TElidedLabel.cpp				\
	$$TOPDIR/lib/widgets/TOkLabel.cpp				\
	$$TOPDIR/lib/widgets/TPathComboBrowse.cpp			\
	$$TOPDIR/src/translator.cpp					\
	$$TOPDIR/src/widgets/confirmationdialog.cpp

RESOURCES +=								\
	$$TOPDIR/lib/resources/lib-resources.qrc			\
	$$TOPDIR/resources/resources.qrc

include(../tests-include.pri)

test_home_prep.commands += ; mkdir -p "$${TEST_HOME}/$${TARGET}";	\
	touch "$${TEST_HOME}/$${TARGET}/empty-file"
