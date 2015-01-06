#-------------------------------------------------
#
# Project created by QtCreator 2014-12-10T01:25:38
#
#-------------------------------------------------

QT       += core gui

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
    mainwindow.cpp

HEADERS  += \
    backuplistwidget.h \
    setupdialog.h \
    tarsnapcli.h \
    jobmanager.h \
    coreapplication.h \
    mainwindow.h

FORMS    += \
    restoreitemwidget.ui \
    backupitemwidget.ui \
    setupdialog.ui \
    mainwindow.ui

RESOURCES += \
    resources.qrc

DISTFILES += \
    COPYING \
    README \
    INSTALL
