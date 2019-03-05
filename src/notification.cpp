#include "notification.h"

WARNINGS_DISABLE
#include <QCoreApplication>
WARNINGS_ENABLE

#include <TSettings.h>

Notification::Notification(QSystemTrayIcon *parent) : QSystemTrayIcon(parent)
{
    setIcon(QIcon(":/logos/tarsnap-icon-big.png"));
}

void Notification::displayNotification(QString message)
{
    TSettings settings;
    if(settings.value("app/notifications", true).toBool()
       && isSystemTrayAvailable())
    {
        // Strip HTML tags
        QString messagePlainText = message.remove(QRegExp("<[^>]*>"));
        // Display message
        show();
        showMessage(QCoreApplication::instance()->applicationName(),
                    messagePlainText);
    }
}
