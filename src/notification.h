#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
#include <QSystemTrayIcon>
WARNINGS_ENABLE

#include "messages/notification_info.h"

/*!
 * \ingroup misc
 * \brief The Notification widget is a QSystemTrayIcon which displays
 * a "pop-up" notification in the user's main desktop.
 */
class Notification : public QSystemTrayIcon
{
    Q_OBJECT

    Q_ENUM(message_type)

public:
    //! Constructor.
    explicit Notification(QSystemTrayIcon *parent = nullptr);

public slots:
    //! Displays a string if the user has "Show desktop notifications" enabled,
    //! after removing HTML tags.
    void displayNotification(const QString &message, enum message_type type,
                             const QString &data);

signals:
    //! The user clicked on the notification message.
    void notification_clicked(enum message_type type, const QString &data);

private slots:
    void handle_messageClicked();

private:
    enum message_type _msg_type;
    QString           _msg_data;
};

#endif // NOTIFICATION_H
