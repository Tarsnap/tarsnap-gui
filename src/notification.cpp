#include "notification.h"

#include <QCoreApplication>
#include <QSettings>

Notification::Notification(QSystemTrayIcon *parent) : QSystemTrayIcon(parent)
{
    setIcon(QIcon(":/icons/tarsnap_icon_big.png"));
}

void Notification::displayNotification(QString message)
{
    QSettings settings;
    if(settings.value("app/notifications", true).toBool() &&
       isSystemTrayAvailable())
    {
        show();
        showMessage(QCoreApplication::instance()->applicationName(),
                    message.remove(QRegExp("<[^>]*>")));
    }
}
