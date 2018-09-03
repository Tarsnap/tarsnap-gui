TARGET = test-mainwindow

FORMS +=							\
	../../forms/aboutwidget.ui				\
	../../forms/archivelistwidgetitem.ui			\
	../../forms/archivewidget.ui				\
	../../forms/backuplistwidgetitem.ui			\
	../../forms/consolewidget.ui				\
	../../forms/filepickerdialog.ui				\
	../../forms/filepickerwidget.ui				\
	../../forms/joblistwidgetitem.ui			\
	../../forms/jobwidget.ui				\
	../../forms/logindialog.ui				\
	../../forms/mainwindow.ui				\
	../../forms/restoredialog.ui

RESOURCES += ../../resources/resources.qrc

HEADERS  +=						\
	../../src/backuptask.h				\
	../../src/customfilesystemmodel.h		\
	../../src/debug.h				\
	../../src/filetablemodel.h			\
	../../src/persistentmodel/archive.h		\
	../../src/persistentmodel/job.h			\
	../../src/persistentmodel/persistentobject.h	\
	../../src/persistentmodel/persistentstore.h	\
	../../src/scheduling.h				\
	../../src/tarsnapaccount.h			\
	../../src/translator.h				\
	../../src/utils.h				\
	../../src/widgets/archivelistwidget.h		\
	../../src/widgets/archivelistwidgetitem.h	\
	../../src/widgets/archivewidget.h		\
	../../src/widgets/backuplistwidget.h		\
	../../src/widgets/backuplistwidgetitem.h	\
	../../src/widgets/busywidget.h			\
	../../src/widgets/elidedlabel.h			\
	../../src/widgets/filepickerdialog.h		\
	../../src/widgets/filepickerwidget.h		\
	../../src/widgets/joblistwidget.h		\
	../../src/widgets/joblistwidgetitem.h		\
	../../src/widgets/jobwidget.h			\
	../../src/widgets/mainwindow.h			\
	../../src/widgets/popuppushbutton.h		\
	../../src/widgets/restoredialog.h		\
	../../tests/qtest-platform.h

SOURCES += test-mainwindow.cpp				\
	../../src/backuptask.cpp			\
	../../src/customfilesystemmodel.cpp		\
	../../src/debug.cpp				\
	../../src/filetablemodel.cpp			\
	../../src/persistentmodel/archive.cpp		\
	../../src/persistentmodel/job.cpp		\
	../../src/persistentmodel/persistentobject.cpp	\
	../../src/persistentmodel/persistentstore.cpp	\
	../../src/scheduling.cpp			\
	../../src/tarsnapaccount.cpp			\
	../../src/translator.cpp			\
	../../src/utils.cpp				\
	../../src/widgets/archivelistwidget.cpp		\
	../../src/widgets/archivelistwidgetitem.cpp	\
	../../src/widgets/archivewidget.cpp		\
	../../src/widgets/backuplistwidget.cpp		\
	../../src/widgets/backuplistwidgetitem.cpp	\
	../../src/widgets/busywidget.cpp		\
	../../src/widgets/elidedlabel.cpp		\
	../../src/widgets/filepickerdialog.cpp		\
	../../src/widgets/filepickerwidget.cpp		\
	../../src/widgets/joblistwidget.cpp		\
	../../src/widgets/joblistwidgetitem.cpp		\
	../../src/widgets/jobwidget.cpp			\
	../../src/widgets/mainwindow.cpp		\
	../../src/widgets/popuppushbutton.cpp		\
	../../src/widgets/restoredialog.cpp

include(../tests-include.pro)
