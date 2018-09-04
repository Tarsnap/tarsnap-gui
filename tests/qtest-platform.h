#ifndef QTEST_PLATFORM_H
#define QTEST_PLATFORM_H

#include <QTest>

#include "utils.h"

// Only relevant for running a gui test with -platform offscreen
#ifdef QT_GUI_LIB
#include <QApplication>

// If we're running with a GUI (i.e. X11), we can watch the app doing tests
#define IF_VISUAL if(QApplication::platformName() != "offscreen")
#define IF_NOT_VISUAL if(!(QApplication::platformName() != "offscreen"))

#define VISUAL_WAIT                                                            \
    do                                                                         \
    {                                                                          \
        IF_VISUAL { QTest::qWait(750); }                                       \
    } while(0)

#define VISUAL_INIT(gui_obj)                                                   \
    IF_VISUAL                                                                  \
    {                                                                          \
        gui_obj->show();                                                       \
        VISUAL_WAIT;                                                           \
    }

// Filter out unwanted messages arising from -platform "offscreen"
QtMessageHandler orig_message_handler;
void offscreenMessageOutput(QtMsgType type, const QMessageLogContext &context,
                            const QString &msg)
{
    switch(type)
    {
    case QtWarningMsg:
        if(msg.contains("Cannot find font directory")
           || msg.contains("This plugin does not support propagateSizeHints()"))
        {
            // Ignore this message.
            return;
        }
    /* FALLTHROUGH */
    default:
        orig_message_handler(type, context, msg);
    }
}
#endif /* end gui-related code */

// Find tarsnap and tarsnap-keygen in $PATH, or skip the test
#define TARSNAP_CLI_OR_SKIP                                                    \
    QString tarsnapPath;                                                       \
    do                                                                         \
    {                                                                          \
        tarsnapPath = Utils::findTarsnapClientInPath(QString(""), true);       \
        if(tarsnapPath.isEmpty())                                              \
            QSKIP("No tarsnap binary found");                                  \
    } while(0)

#endif
