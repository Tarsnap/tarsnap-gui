TARGET = test-mainwindow

FORMS +=							\
	../../forms/backuplistwidgetitem.ui			\
	../../forms/consolewidget.ui				\
	../../forms/mainwindow.ui				\
	../../forms/restoredialog.ui				\
	../../forms/filepickerdialog.ui				\
	../../forms/filepickerwidget.ui				\
	../../forms/jobwidget.ui				\
	../../forms/aboutwidget.ui				\
	../../forms/logindialog.ui				\
	../../forms/archivewidget.ui				\
	../../forms/archivelistwidgetitem.ui			\
	../../forms/joblistwidgetitem.ui

RESOURCES += ../../resources/resources.qrc

HEADERS  +=						\
	../../tests/qtest-platform.h			\
	../../src/debug.h				\
	../../src/utils.h				\
	../../src/widgets/backuplistwidget.h		\
	../../src/widgets/backuplistwidgetitem.h	\
	../../src/widgets/archivelistwidget.h		\
	../../src/widgets/archivelistwidgetitem.h	\
	../../src/widgets/filepickerdialog.h		\
	../../src/widgets/filepickerwidget.h		\
	../../src/widgets/joblistwidget.h		\
	../../src/widgets/joblistwidgetitem.h		\
	../../src/widgets/jobwidget.h			\
	../../src/widgets/mainwindow.h			\
	../../src/widgets/restoredialog.h		\
	../../src/widgets/popuppushbutton.h		\
	../../src/widgets/elidedlabel.h			\
	../../src/widgets/busywidget.h			\
	../../src/customfilesystemmodel.h		\
	../../src/scheduling.h				\
	../../src/taskmanager.h				\
	../../src/tarsnaptask.h				\
	../../src/persistentmodel/persistentstore.h	\
	../../src/persistentmodel/persistentobject.h	\
	../../src/persistentmodel/archive.h		\
	../../src/persistentmodel/job.h			\
	../../src/backuptask.h				\
	../../src/tarsnapaccount.h			\
	../../src/notification.h			\
	../../src/error.h				\
	../../src/persistentmodel/journal.h		\
	../../src/widgets/archivewidget.h		\
	../../src/filetablemodel.h			\
	../../src/translator.h

SOURCES += test-mainwindow.cpp				\
	../../src/debug.cpp				\
	../../src/utils.cpp				\
	../../src/widgets/backuplistwidget.cpp		\
	../../src/widgets/backuplistwidgetitem.cpp	\
	../../src/widgets/archivelistwidget.cpp		\
	../../src/widgets/archivelistwidgetitem.cpp	\
	../../src/widgets/filepickerdialog.cpp		\
	../../src/widgets/filepickerwidget.cpp		\
	../../src/widgets/joblistwidget.cpp		\
	../../src/widgets/joblistwidgetitem.cpp		\
	../../src/widgets/jobwidget.cpp			\
	../../src/widgets/mainwindow.cpp		\
	../../src/widgets/popuppushbutton.cpp		\
	../../src/widgets/restoredialog.cpp		\
	../../src/widgets/elidedlabel.cpp		\
	../../src/widgets/busywidget.cpp		\
	../../src/customfilesystemmodel.cpp		\
	../../src/scheduling.cpp			\
	../../src/taskmanager.cpp			\
	../../src/tarsnaptask.cpp			\
	../../src/persistentmodel/persistentstore.cpp	\
	../../src/persistentmodel/persistentobject.cpp	\
	../../src/persistentmodel/archive.cpp		\
	../../src/persistentmodel/job.cpp		\
	../../src/backuptask.cpp			\
	../../src/tarsnapaccount.cpp			\
	../../src/notification.cpp			\
	../../src/persistentmodel/journal.cpp		\
	../../src/widgets/archivewidget.cpp		\
	../../src/filetablemodel.cpp			\
	../../src/translator.cpp

include(../tests-include.pro)
