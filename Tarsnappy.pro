#-------------------------------------------------
#
# Project created by QtCreator 2014-12-10T01:25:38
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Tarsnappy
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp\
    backuplistwidget.cpp \
    setupdialog.cpp \
    tarsnapcli.cpp \
    jobmanager.cpp \
    coreapplication.cpp \
    mainwindow.cpp \
    backuplistitem.cpp \
    textlabel.cpp \
    utils.cpp \
    browselistwidget.cpp \
    archivelistitem.cpp \
    debug.cpp \
    restoredialog.cpp \
    filepicker.cpp \
    filepickerdialog.cpp \
    joblistitem.cpp \
    jobslistwidget.cpp

HEADERS  += \
    backuplistwidget.h \
    setupdialog.h \
    tarsnapcli.h \
    jobmanager.h \
    coreapplication.h \
    mainwindow.h \
    backuplistitem.h \
    textlabel.h \
    utils.h \
    browselistwidget.h \
    archivelistitem.h \
    debug.h \
    restoredialog.h \
    filepicker.h \
    filepickerdialog.h \
    joblistitem.h \
    jobslistwidget.h

FORMS    += \
    backupitemwidget.ui \
    setupdialog.ui \
    mainwindow.ui \
    archiveitemwidget.ui \
    restoredialog.ui \
    filepicker.ui \
    filepickerdialog.ui \
    jobwidget.ui \
    jobitemwidget.ui

RESOURCES += \
    resources.qrc

DISTFILES += \
    COPYING \
    README \
    INSTALL \
    CHANGELOG

osx {
    OBJECTIVE_SOURCES += platform_osx.mm
    LIBS += -framework Foundation

    ICON = resources/tarsnap.icns
}
