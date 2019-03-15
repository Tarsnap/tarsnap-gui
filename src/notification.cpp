#include "notification.h"

WARNINGS_DISABLE
#include <QCoreApplication>
WARNINGS_ENABLE

#include <TSettings.h>

Notification::Notification(QSystemTrayIcon *parent) : QSystemTrayIcon(parent)
{
    setIcon(QIcon(":/logos/tarsnap-icon-big.png"));
    connect(this, &QSystemTrayIcon::messageClicked, this,
            &Notification::handle_messageClicked);
}

void Notification::displayNotification(QString message, enum message_type type,
                                       QString data)
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
        // Store info for later handling
        _msg_type = type;
        _msg_data = data;
    }
}

void Notification::handle_messageClicked()
{
}
