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

VERSION = 1.0.2
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
#QMAKE_TARGET_COMPANY = Tarsnap Backup Inc.
#QMAKE_TARGET_PRODUCT = Tarsnap
#QMAKE_TARGET_DESCRIPTION = GUI frontend for Tarsnap
#QMAKE_TARGET_COPYRIGHT = copyright Tarsnap Backup Inc.

SOURCES +=						\
	lib/util/optparse.c				\
	lib/util/optparse_helper.c			\
	libcperciva/util/getopt.c			\
	libcperciva/util/warnp.c			\
	src/backuptask.cpp				\
	src/coreapplication.cpp				\
	src/customfilesystemmodel.cpp			\
	src/debug.cpp					\
	src/filetablemodel.cpp				\
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
	src/widgets/archivewidget.cpp			\
	src/widgets/backuplistwidget.cpp		\
	src/widgets/backuplistwidgetitem.cpp		\
	src/widgets/busywidget.cpp			\
	src/widgets/elidedlabel.cpp			\
	src/widgets/filepickerdialog.cpp		\
	src/widgets/filepickerwidget.cpp		\
	src/widgets/joblistwidget.cpp			\
	src/widgets/joblistwidgetitem.cpp		\
	src/widgets/jobwidget.cpp			\
	src/widgets/mainwindow.cpp			\
	src/widgets/popuppushbutton.cpp			\
	src/widgets/restoredialog.cpp			\
	src/widgets/setupdialog.cpp

HEADERS +=						\
	lib/util/optparse.h				\
	lib/util/optparse_helper.h			\
	libcperciva/util/getopt.h			\
	libcperciva/util/warnp.h			\
	src/backuptask.h				\
	src/coreapplication.h				\
	src/customfilesystemmodel.h			\
	src/debug.h					\
	src/filetablemodel.h				\
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
	src/widgets/archivewidget.h			\
	src/widgets/backuplistwidget.h			\
	src/widgets/backuplistwidgetitem.h		\
	src/widgets/busywidget.h			\
	src/widgets/elidedlabel.h			\
	src/widgets/filepickerdialog.h			\
	src/widgets/filepickerwidget.h			\
	src/widgets/joblistwidget.h			\
	src/widgets/joblistwidgetitem.h			\
	src/widgets/jobwidget.h				\
	src/widgets/mainwindow.h			\
	src/widgets/popuppushbutton.h			\
	src/widgets/restoredialog.h			\
	src/widgets/setupdialog.h

INCLUDEPATH += src/widgets/				\
		+= libcperciva/util/			\
		+= lib/util/				\
		+= src/

FORMS +=						\
	forms/aboutwidget.ui				\
	forms/archivelistwidgetitem.ui			\
	forms/archivewidget.ui				\
	forms/backuplistwidgetitem.ui			\
	forms/consolewidget.ui				\
	forms/filepickerdialog.ui			\
	forms/filepickerwidget.ui			\
	forms/joblistwidgetitem.ui			\
	forms/jobwidget.ui				\
	forms/logindialog.ui				\
	forms/mainwindow.ui				\
	forms/restoredialog.ui				\
	forms/setupdialog.ui

RESOURCES += resources/resources.qrc

DISTFILES +=						\
	CHANGELOG					\
	COPYING						\
	INSTALL						\
	README

DISTFILES += .clang-format

TRANSLATIONS = resources/translations/tarsnap-gui_en.ts \
               resources/translations/tarsnap-gui_ro.ts

# Cleaner source directory
UI_DIR      = build/gui/
MOC_DIR     = build/gui/
RCC_DIR     = build/gui/
OBJECTS_DIR = build/gui/

# Start off with tests which require the most compilation units, in order
# to maximize the benefits of parallel builds.
UNIT_TESTS =						\
	tests/mainwindow				\
	tests/setupwizard				\
	tests/taskmanager				\
	tests/customfilesystemmodel			\
	tests/task

osx {
    LIBS += -framework Foundation
    ICON = resources/logos/tarsnap.icns
    TARGET = Tarsnap

    # Add VERSION to the app bundle.  (I wish that qmake did this!)
    INFO_PLIST_PATH = $$shell_quote($${OUT_PWD}/$${TARGET}.app/Contents/Info.plist)
    QMAKE_POST_LINK += /usr/libexec/PlistBuddy -c \"Set :CFBundleGetInfoString $${VERSION}\" $${INFO_PLIST_PATH} ;
}

format.commands = find src/ tests/ -name \"*.h\" -or -name \"*.cpp\" |	\
			xargs clang-format -i
QMAKE_EXTRA_TARGETS += format

# The same variable is used in individual tests
TEST_HOME = /tmp/tarsnap-gui-test
test_home_prep.commands = @rm -rf "$${TEST_HOME}"

test.commands =		@echo "Compiling tests...";			\
			for D in $${UNIT_TESTS}; do			\
				(cd \$\${D} && \${QMAKE} && \${MAKE} -s); \
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
