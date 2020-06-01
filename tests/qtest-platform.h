#ifndef QTEST_PLATFORM_H
#define QTEST_PLATFORM_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QEventLoop>
#include <QTest>

#ifdef QT_GUI_LIB
#include <QApplication>
#endif
WARNINGS_ENABLE

#include "dir-utils.h"

// Only relevant for running a gui test with platform=offscreen
#ifdef QT_GUI_LIB

// If we're running with a GUI (i.e. X11), we can watch the app doing tests
#define IF_VISUAL if(QApplication::platformName() != "offscreen")
#define IF_NOT_VISUAL if(!(QApplication::platformName() != "offscreen"))

#define VISUAL_WAIT                                                            \
    do                                                                         \
    {                                                                          \
        IF_VISUAL { QTest::qWait(750); }                                       \
    } while(0)

#define VISUAL_INIT(gui_obj)                                                   \
    do                                                                         \
    {                                                                          \
        IF_VISUAL                                                              \
        {                                                                      \
            gui_obj->show();                                                   \
            VISUAL_WAIT;                                                       \
        }                                                                      \
        else { gui_obj->show(); }                                              \
    } while(0)

// Filter out unwanted messages arising from platform="offscreen"
void offscreenMessageOutput(QtMsgType type, const QMessageLogContext &context,
                            const QString &msg);

static const QtMessageHandler orig_message_handler =
    qInstallMessageHandler(nullptr);
void offscreenMessageOutput(QtMsgType type, const QMessageLogContext &context,
                            const QString &msg)
{
    // Ignore messages arising from the testing platform
    if(type == QtWarningMsg)
    {
        if(msg.contains("Cannot find font directory")
           || msg.contains("This plugin does not support propagateSizeHints()"))
        {
            // Ignore this message.
            return;
        }
    }

    // Handle other messages
    orig_message_handler(type, context, msg);
}
#endif /* end gui-related code */

// Find tarsnap and tarsnap-keygen in $PATH, or skip the test
#define TARSNAP_CLI_OR_SKIP                                                    \
    QString tarsnapPath;                                                       \
    do                                                                         \
    {                                                                          \
        struct DirMessage result = findTarsnapClientInPath(QString(""), true); \
        tarsnapPath              = result.dirname;                             \
        WARNINGS_DISABLE                                                       \
        if(tarsnapPath.isEmpty())                                              \
            QSKIP("No tarsnap binary found");                                  \
        WARNINGS_ENABLE                                                        \
    } while(0)

// Deal with not being able to change a widget's text and signal the change
#define SET_TEXT_WITH_SIGNAL(WIDGET, TEXT)                                     \
    WIDGET->setText(TEXT);                                                     \
    QMetaObject::invokeMethod(WIDGET, "editingFinished", Qt::QueuedConnection)

// Deal with platforms which ignore $XDG_*_HOME environment vars
#if defined(Q_OS_MACOS)
#define HANDLE_IGNORING_XDG_HOME                                               \
    do                                                                         \
    {                                                                          \
        TSettings::setFilename(TEST_DIR "/" TEST_NAME ".conf");                \
    } while(0)
#else
#define HANDLE_IGNORING_XDG_HOME
#endif

QString get_script(const QString &scriptname);
QString get_script(const QString &scriptname)
{
    QDir dir = QDir(QCoreApplication::applicationDirPath());

#ifdef Q_OS_OSX
    // Handle the OSX app bundle.
    dir.cdUp();
    dir.cdUp();
    dir.cdUp();
#endif

    return dir.absolutePath() + "/" + scriptname;
}

#define WAIT_FINAL QCoreApplication::processEvents(QEventLoop::AllEvents, 5000)
#define WAIT_UNTIL(x)                                                          \
    while(!(x))                                                                \
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100)
#define WAIT_SIG(x) WAIT_UNTIL(x.count() > 0)

#endif
