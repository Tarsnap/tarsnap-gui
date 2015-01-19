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
    utils.cpp

HEADERS  += \
    backuplistwidget.h \
    setupdialog.h \
    tarsnapcli.h \
    jobmanager.h \
    coreapplication.h \
    mainwindow.h \
    backuplistitem.h \
    textlabel.h \
    utils.h

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

osx {
    OBJECTIVE_SOURCES += platform_osx.mm
    LIBS += -framework Foundation
}
