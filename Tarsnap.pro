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
env_CC = $$(QMAKE_CC)
!isEmpty(env_CC) {
	QMAKE_CC = $$(QMAKE_CC)
}
env_CXX = $$(QMAKE_CXX)
!isEmpty(env_CXX) {
	QMAKE_CXX = $$(QMAKE_CXX)
}
env_LINK = $$(QMAKE_LINK)
!isEmpty(env_LINK) {
	QMAKE_LINK = $$(QMAKE_LINK)
}

#QMAKE_TARGET_COMPANY = Tarsnap Backup Inc.
#QMAKE_TARGET_PRODUCT = Tarsnap
#QMAKE_TARGET_DESCRIPTION = GUI frontend for Tarsnap
#QMAKE_TARGET_COPYRIGHT = copyright Tarsnap Backup Inc.

SOURCES +=						\
	lib/core/ConsoleLog.cpp				\
	lib/core/TSettings.cpp				\
	lib/util/optparse.c				\
	lib/util/optparse_helper.c			\
	lib/widgets/TBusyLabel.cpp			\
	lib/widgets/TElidedLabel.cpp			\
	lib/widgets/TOkLabel.cpp			\
	lib/widgets/TPathComboBrowse.cpp		\
	lib/widgets/TPathLineBrowse.cpp			\
	lib/widgets/TPopupPushButton.cpp		\
	lib/widgets/TTextView.cpp			\
	lib/widgets/TWizard.cpp				\
	lib/widgets/TWizardPage.cpp			\
	libcperciva/util/getopt.c			\
	libcperciva/util/warnp.c			\
	src/app-cmdline.cpp				\
	src/app-gui.cpp					\
	src/app-setup.cpp				\
	src/backenddata.cpp				\
	src/backuptask.cpp				\
	src/basetask.cpp				\
	src/cmdlinetask.cpp				\
	src/customfilesystemmodel.cpp			\
	src/dir-utils.cpp				\
	src/dirinfotask.cpp				\
	src/filetablemodel.cpp				\
	src/humanbytes.cpp				\
	src/init-shared.cpp				\
	src/jobrunner.cpp				\
	src/main.cpp					\
	src/notification.cpp				\
	src/parsearchivelistingtask.cpp			\
	src/persistentmodel/archive.cpp			\
	src/persistentmodel/job.cpp			\
	src/persistentmodel/journal.cpp			\
	src/persistentmodel/persistentobject.cpp	\
	src/persistentmodel/persistentstore.cpp		\
	src/persistentmodel/upgrade-store.cpp		\
	src/scheduling.cpp				\
	src/setupwizard/setupwizard.cpp			\
	src/setupwizard/setupwizard_cli.cpp		\
	src/setupwizard/setupwizard_final.cpp		\
	src/setupwizard/setupwizard_intro.cpp		\
	src/setupwizard/setupwizard_register.cpp	\
	src/tarsnapaccount.cpp				\
	src/taskmanager.cpp				\
	src/taskqueuer.cpp				\
	src/tasks/tasks-misc.cpp			\
	src/tasks/tasks-setup.cpp			\
	src/tasks/tasks-tarsnap.cpp			\
	src/tasks/tasks-utils.cpp			\
	src/translator.cpp				\
	src/widgets/archivelistwidget.cpp		\
	src/widgets/archivelistwidgetitem.cpp		\
	src/widgets/archivestabwidget.cpp		\
	src/widgets/archivewidget.cpp			\
	src/widgets/backuplistwidget.cpp		\
	src/widgets/backuplistwidgetitem.cpp		\
	src/widgets/backuptabwidget.cpp			\
	src/widgets/confirmationdialog.cpp		\
	src/widgets/elidedannotatedlabel.cpp		\
	src/widgets/elidedclickablelabel.cpp		\
	src/widgets/filepickerdialog.cpp		\
	src/widgets/filepickerwidget.cpp		\
	src/widgets/helpwidget.cpp			\
	src/widgets/joblistwidget.cpp			\
	src/widgets/joblistwidgetitem.cpp		\
	src/widgets/jobstabwidget.cpp			\
	src/widgets/jobwidget.cpp			\
	src/widgets/mainwindow.cpp			\
	src/widgets/restoredialog.cpp			\
	src/widgets/schedulingwidgets.cpp		\
	src/widgets/settingswidget.cpp			\
	src/widgets/statisticsdialog.cpp		\
	src/widgets/statusbarwidget.cpp			\
	src/widgets/stoptasksdialog.cpp			\
	src/widgets/tarsnapaccountdialog.cpp

HEADERS +=						\
	lib/core/ConsoleLog.h				\
	lib/core/LogEntry.h				\
	lib/core/TSettings.h				\
	lib/core/warnings-disable.h			\
	lib/util/optparse.h				\
	lib/util/optparse_helper.h			\
	lib/widgets/TBusyLabel.h			\
	lib/widgets/TElidedLabel.h			\
	lib/widgets/TOkLabel.h				\
	lib/widgets/TPathComboBrowse.h			\
	lib/widgets/TPathLineBrowse.h			\
	lib/widgets/TPopupPushButton.h			\
	lib/widgets/TTextView.h				\
	lib/widgets/TWizard.h				\
	lib/widgets/TWizardPage.h			\
	libcperciva/util/getopt.h			\
	libcperciva/util/warnp.h			\
	src/app-cmdline.h				\
	src/app-gui.h					\
	src/app-setup.h					\
	src/backenddata.h				\
	src/backuptask.h				\
	src/basetask.h					\
	src/compat.h					\
	src/cmdlinetask.h				\
	src/customfilesystemmodel.h			\
	src/debug.h					\
	src/dir-utils.h					\
	src/dirinfotask.h				\
	src/filetablemodel.h				\
	src/humanbytes.h				\
	src/init-shared.h				\
	src/jobrunner.h					\
	src/messages/archivefilestat.h			\
	src/messages/archiveptr.h			\
	src/messages/archiverestoreoptions.h		\
	src/messages/backuptaskdataptr.h		\
	src/messages/jobptr.h				\
	src/messages/notification_info.h		\
	src/messages/tarsnaperror.h			\
	src/messages/taskstatus.h			\
	src/notification.h				\
	src/parsearchivelistingtask.h			\
	src/persistentmodel/archive.h			\
	src/persistentmodel/job.h			\
	src/persistentmodel/journal.h			\
	src/persistentmodel/persistentobject.h		\
	src/persistentmodel/persistentstore.h		\
	src/persistentmodel/upgrade-store.h		\
	src/scheduling.h				\
	src/setupwizard/setupwizard.h			\
	src/setupwizard/setupwizard_cli.h		\
	src/setupwizard/setupwizard_final.h		\
	src/setupwizard/setupwizard_intro.h		\
	src/setupwizard/setupwizard_register.h		\
	src/tarsnapaccount.h				\
	src/taskmanager.h				\
	src/taskqueuer.h				\
	src/tasks/tasks-defs.h				\
	src/tasks/tasks-misc.h				\
	src/tasks/tasks-setup.h				\
	src/tasks/tasks-tarsnap.h			\
	src/tasks/tasks-utils.h				\
	src/translator.h				\
	src/widgets/archivelistwidget.h			\
	src/widgets/archivelistwidgetitem.h		\
	src/widgets/archivestabwidget.h			\
	src/widgets/archivewidget.h			\
	src/widgets/backuplistwidget.h			\
	src/widgets/backuplistwidgetitem.h		\
	src/widgets/backuptabwidget.h			\
	src/widgets/confirmationdialog.h		\
	src/widgets/elidedannotatedlabel.h		\
	src/widgets/elidedclickablelabel.h		\
	src/widgets/filepickerdialog.h			\
	src/widgets/filepickerwidget.h			\
	src/widgets/helpwidget.h			\
	src/widgets/joblistwidget.h			\
	src/widgets/joblistwidgetitem.h			\
	src/widgets/jobstabwidget.h			\
	src/widgets/jobwidget.h				\
	src/widgets/mainwindow.h			\
	src/widgets/restoredialog.h			\
	src/widgets/schedulingwidgets.h			\
	src/widgets/settingswidget.h			\
	src/widgets/statisticsdialog.h			\
	src/widgets/statusbarwidget.h			\
	src/widgets/stoptasksdialog.h			\
	src/widgets/tarsnapaccountdialog.h

INCLUDEPATH +=						\
	lib/core/					\
	lib/util/					\
	lib/widgets/					\
	libcperciva/util/				\
	src/

FORMS +=						\
	forms/aboutwidget.ui				\
	forms/archivelistwidgetitem.ui			\
	forms/archivestabwidget.ui			\
	forms/archivewidget.ui				\
	forms/backuplistwidgetitem.ui			\
	forms/backuptabwidget.ui			\
	forms/consolewidget.ui				\
	forms/filepickerdialog.ui			\
	forms/filepickerwidget.ui			\
	forms/helpwidget.ui				\
	forms/joblistwidgetitem.ui			\
	forms/jobstabwidget.ui				\
	forms/jobwidget.ui				\
	forms/logindialog.ui				\
	forms/mainwindow.ui				\
	forms/restoredialog.ui				\
	forms/settingswidget.ui				\
	forms/setupwizard.ui				\
	forms/setupwizard_cli.ui			\
	forms/setupwizard_final.ui			\
	forms/setupwizard_intro.ui			\
	forms/setupwizard_register.ui			\
	forms/statisticsdialog.ui			\
	forms/statusbarwidget.ui			\
	forms/stoptasksdialog.ui			\
	lib/forms/TPathComboBrowse.ui			\
	lib/forms/TPathLineBrowse.ui			\
	lib/forms/TWizard.ui

RESOURCES +=						\
	lib/resources/lib-resources.qrc			\
	resources/resources.qrc

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
	tests/app-cmdline				\
	tests/app-setup					\
	tests/translations				\
	tests/jobstabwidget				\
	tests/settingswidget				\
	tests/backuptabwidget				\
	tests/archivestabwidget				\
	tests/helpwidget				\
	tests/persistent				\
	tests/setupwizard				\
	tests/taskmanager				\
	tests/customfilesystemmodel			\
	tests/small-widgets				\
	tests/lib-widgets				\
	tests/consolelog				\
	tests/task					\
	tests/core

OPTIONAL_BUILD_ONLY_TESTS = tests/cli

osx {
    LIBS += -framework Foundation
    ICON = resources/logos/tarsnap.icns
    TARGET = Tarsnap

    # Add VERSION to the app bundle.  (I wish that qmake did this!)
    INFO_PLIST_PATH = $$shell_quote($${OUT_PWD}/$${TARGET}.app/Contents/Info.plist)
    QMAKE_POST_LINK += /usr/libexec/PlistBuddy -c \"Set :CFBundleGetInfoString $${VERSION}\" $${INFO_PLIST_PATH} ;
}

format.commands = find src/ tests/ lib/core/ lib/widgets/ lib/plugins	\
			-name \"*.h\" -or -name \"*.cpp\" |		\
			xargs clang-format13 -i
update_translations.commands = lupdate -locations none -no-obsolete Tarsnap.pro
QMAKE_EXTRA_TARGETS += format update_translations

# The same variable is used in individual tests
TEST_HOME = /tmp/tarsnap-gui-test
test_home_prep.commands = @rm -rf "$${TEST_HOME}"

# Prep the tests
buildtests = $$UNIT_TESTS $$BUILD_ONLY_TESTS
for(D, buildtests) {
	cmd=	cd $${D} &&					\
			CFLAGS=\"$$(CFLAGS)\"			\
			CXXFLAGS=\"$$(CXXFLAGS)\"		\
			LDFLAGS=\"$$(LDFLAGS)\"			\
			QMAKE_CC=\"$${QMAKE_CC}\"		\
			QMAKE_CXX=\"$${QMAKE_CXX}\"		\
			QMAKE_LINK=\"$${QMAKE_LINK}\"		\
			$${QMAKE_QMAKE} -spec $${QMAKESPEC}
	system($$cmd)|error("Failed to qmake in: $$D")
}

test.commands =		@echo "Compiling tests...";			\
			for D in $${UNIT_TESTS} $${BUILD_ONLY_TESTS}; do \
				(cd \$\${D} && \${MAKE} -s);		\
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

# Prep the optional tests
optional_buildtests = $$OPTIONAL_BUILD_ONLY_TESTS
for(D, optional_buildtests) {
	cmd=	cd $${D} &&					\
			CFLAGS=\"$$(CFLAGS)\"			\
			CXXFLAGS=\"$$(CXXFLAGS)\"		\
			LDFLAGS=\"$$(LDFLAGS)\"			\
			QMAKE_CC=\"$${QMAKE_CC}\"		\
			QMAKE_CXX=\"$${QMAKE_CXX}\"		\
			QMAKE_LINK=\"$${QMAKE_LINK}\"		\
			$${QMAKE_QMAKE} -spec $${QMAKESPEC}
	system($$cmd)|error("Failed to qmake in: $$D")
}

optional_buildtest.commands =	@echo "Compiling optional tests...";	\
			for D in $${OPTIONAL_BUILD_ONLY_TESTS}; do	\
				(cd \$\${D} && \${MAKE} -s);		\
				err=\$\$?;				\
				if \[ \$\${err} -gt "0" \]; then	\
					exit \$\${err};			\
				fi;					\
			done;						\

# Yes, this also does distclean
test_clean.commands =	for D in $${UNIT_TESTS} $${OPTIONAL_BUILD_ONLY_TESTS}; do	\
				(cd \$\${D} && \${QMAKE} &&		\
				    \${MAKE} distclean);		\
			done
clean.depends += test_clean

QMAKE_EXTRA_TARGETS += test test_clean clean test_home_prep optional_buildtest
