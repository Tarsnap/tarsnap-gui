# Check for at least Qt 5.2
NOTSUPPORTED=0
lessThan(QT_MAJOR_VERSION, 6) {
    lessThan(QT_MAJOR_VERSION, 5) {
        # Qt 4 or below
        NOTSUPPORTED=1
    } else {
        lessThan(QT_MINOR_VERSION, 2) {
            # Qt 5.0 or 5.1
             NOTSUPPORTED=1
        }
    }
}
greaterThan(NOTSUPPORTED, 0) {
    error("Tarsnap-gui requires Qt 5.2 or higher; found $${QT_VERSION}.")
}

QT += core gui network sql widgets
CONFIG += c++11

TEMPLATE = app
TARGET = tarsnap-gui

VERSION = 1.1.0-unreleased
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
DEFINES += QT_NO_FOREACH

# Pick up extra flags from the environment
QMAKE_CXXFLAGS += $$(CXXFLAGS)
QMAKE_CFLAGS += $$(CFLAGS)
QMAKE_LFLAGS += $$(LDFLAGS)

#QMAKE_TARGET_COMPANY = Tarsnap Backup Inc.
#QMAKE_TARGET_PRODUCT = Tarsnap
#QMAKE_TARGET_DESCRIPTION = GUI frontend for Tarsnap
#QMAKE_TARGET_COPYRIGHT = copyright Tarsnap Backup Inc.

SOURCES +=						\
	lib/core/TSettings.cpp				\
	lib/util/optparse.c				\
	lib/util/optparse_helper.c			\
	libcperciva/util/getopt.c			\
	libcperciva/util/warnp.c			\
	src/app-cmdline.cpp				\
	src/app-gui.cpp					\
	src/backuptask.cpp				\
	src/customfilesystemmodel.cpp			\
	src/debug.cpp					\
	src/filetablemodel.cpp				\
	src/init-shared.cpp				\
	src/main.cpp					\
	src/notification.cpp				\
	src/persistentmodel/archive.cpp			\
	src/persistentmodel/job.cpp			\
	src/persistentmodel/journal.cpp			\
	src/persistentmodel/persistentobject.cpp	\
	src/persistentmodel/persistentstore.cpp		\
	src/scheduling.cpp				\
	src/tarsnapaccount.cpp				\
	src/tarsnaptask.cpp				\
	src/taskmanager.cpp				\
	src/translator.cpp				\
	src/utils.cpp					\
	src/widgets/archivelistwidget.cpp		\
	src/widgets/archivelistwidgetitem.cpp		\
	src/widgets/archivestabwidget.cpp		\
	src/widgets/archivewidget.cpp			\
	src/widgets/backuplistwidget.cpp		\
	src/widgets/backuplistwidgetitem.cpp		\
	src/widgets/busywidget.cpp			\
	src/widgets/confirmationdialog.cpp		\
	src/widgets/elidedlabel.cpp			\
	src/widgets/filepickerdialog.cpp		\
	src/widgets/filepickerwidget.cpp		\
	src/widgets/helpwidget.cpp			\
	src/widgets/joblistwidget.cpp			\
	src/widgets/joblistwidgetitem.cpp		\
	src/widgets/jobwidget.cpp			\
	src/widgets/mainwindow.cpp			\
	src/widgets/popuppushbutton.cpp			\
	src/widgets/restoredialog.cpp			\
	src/widgets/settingswidget.cpp			\
	src/widgets/setupdialog.cpp

HEADERS +=						\
	lib/core/TSettings.h				\
	lib/util/optparse.h				\
	lib/util/optparse_helper.h			\
	libcperciva/util/getopt.h			\
	libcperciva/util/warnp.h			\
	src/app-cmdline.h				\
	src/app-gui.h					\
	src/backuptask.h				\
	src/customfilesystemmodel.h			\
	src/debug.h					\
	src/filetablemodel.h				\
	src/init-shared.h				\
	src/notification.h				\
	src/persistentmodel/archive.h			\
	src/persistentmodel/job.h			\
	src/persistentmodel/journal.h			\
	src/persistentmodel/persistentobject.h		\
	src/persistentmodel/persistentstore.h		\
	src/scheduling.h				\
	src/tarsnapaccount.h				\
	src/tarsnaperror.h				\
	src/tarsnaptask.h				\
	src/taskmanager.h				\
	src/taskstatus.h				\
	src/translator.h				\
	src/utils.h					\
	src/widgets/archivelistwidget.h			\
	src/widgets/archivelistwidgetitem.h		\
	src/widgets/archivestabwidget.h			\
	src/widgets/archivewidget.h			\
	src/widgets/backuplistwidget.h			\
	src/widgets/backuplistwidgetitem.h		\
	src/widgets/busywidget.h			\
	src/widgets/confirmationdialog.h		\
	src/widgets/elidedlabel.h			\
	src/widgets/filepickerdialog.h			\
	src/widgets/filepickerwidget.h			\
	src/widgets/helpwidget.h			\
	src/widgets/joblistwidget.h			\
	src/widgets/joblistwidgetitem.h			\
	src/widgets/jobwidget.h				\
	src/widgets/mainwindow.h			\
	src/widgets/popuppushbutton.h			\
	src/widgets/restoredialog.h			\
	src/widgets/settingswidget.h			\
	src/widgets/setupdialog.h

INCLUDEPATH += src/widgets/				\
		+= libcperciva/util/			\
		+= lib/core/				\
		+= lib/util/				\
		+= src/

FORMS +=						\
	forms/aboutwidget.ui				\
	forms/archivelistwidgetitem.ui			\
	forms/archivestabwidget.ui			\
	forms/archivewidget.ui				\
	forms/backuplistwidgetitem.ui			\
	forms/consolewidget.ui				\
	forms/filepickerdialog.ui			\
	forms/filepickerwidget.ui			\
	forms/helpwidget.ui				\
	forms/joblistwidgetitem.ui			\
	forms/jobwidget.ui				\
	forms/logindialog.ui				\
	forms/mainwindow.ui				\
	forms/restoredialog.ui				\
	forms/settingswidget.ui				\
	forms/setupdialog.ui

RESOURCES += resources/resources.qrc

DISTFILES +=						\
	CHANGELOG					\
	COPYING						\
	INSTALL						\
	README

DISTFILES += .clang-format

# Handle translations
TRANSLATIONS = resources/translations/tarsnap-gui_en.ts \
               resources/translations/tarsnap-gui_ro.ts

qtPrepareTool(LRELEASE, lrelease)
for(tsfile, TRANSLATIONS) {
	qmfile = $$shadowed($$tsfile)
	qmfile ~= s,.ts$,.qm,
	qmdir = $$dirname(qmfile)
	command = $$LRELEASE -removeidentical $$tsfile -qm $$qmfile
	system($$command)|error("Failed to run: $$command")
}

# Cleaner source directory
UI_DIR      = build/gui/
MOC_DIR     = build/gui/
RCC_DIR     = build/gui/
OBJECTS_DIR = build/gui/

# Start off with tests which require the most compilation units, in order
# to maximize the benefits of parallel builds.
UNIT_TESTS =						\
	tests/mainwindow				\
	tests/cmdline					\
	tests/settingswidget				\
	tests/setupwizard				\
	tests/taskmanager				\
	tests/customfilesystemmodel			\
	tests/task

BUILD_ONLY_TESTS = tests/cli

osx {
    LIBS += -framework Foundation
    ICON = resources/logos/tarsnap.icns
    TARGET = Tarsnap

    # Add VERSION to the app bundle.  (I wish that qmake did this!)
    INFO_PLIST_PATH = $$shell_quote($${OUT_PWD}/$${TARGET}.app/Contents/Info.plist)
    QMAKE_POST_LINK += /usr/libexec/PlistBuddy -c \"Set :CFBundleGetInfoString $${VERSION}\" $${INFO_PLIST_PATH} ;
}

format.commands = find src/ tests/ lib/core/				\
			-name \"*.h\" -or -name \"*.cpp\" |		\
			xargs clang-format -i
update_translations.commands = lupdate -locations none -no-obsolete Tarsnap.pro
QMAKE_EXTRA_TARGETS += format update_translations

# The same variable is used in individual tests
TEST_HOME = /tmp/tarsnap-gui-test
test_home_prep.commands = @rm -rf "$${TEST_HOME}"

test.commands =		@echo "Compiling tests...";			\
			for D in $${UNIT_TESTS} $${BUILD_ONLY_TESTS}; do \
				(cd \$\${D} &&				\
					CFLAGS=\"$$(CFLAGS)\"		\
					CXXFLAGS=\"$$(CXXFLAGS)\"	\
					LDFLAGS=\"$$(LDFLAGS)\"		\
					\${QMAKE} -spec $${QMAKESPEC}	\
						QMAKE_CC=\"$${QMAKE_CC}\" \
						QMAKE_CXX=\"$${QMAKE_CXX}\" \
					&& \${MAKE} -s);		\
				err=\$\$?;				\
				if \[ \$\${err} -gt "0" \]; then	\
					exit \$\${err};			\
				fi;					\
			done;						\
			echo "Running tests...";			\
			for D in $${UNIT_TESTS}; do			\
				(cd \$\${D} && \${MAKE} test -s);	\
				err=\$\$?;				\
				if \[ \$\${err} -gt "0" \]; then	\
					exit \$\${err};			\
				fi;					\
			done
test.depends = test_home_prep

# Yes, this also does distclean
test_clean.commands =	for D in $${UNIT_TESTS}; do			\
				(cd \$\${D} && \${QMAKE} &&		\
				    \${MAKE} distclean);		\
			done
clean.depends += test_clean

QMAKE_EXTRA_TARGETS += test test_clean clean test_home_prep
