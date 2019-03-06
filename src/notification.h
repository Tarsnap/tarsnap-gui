#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
#include <QSystemTrayIcon>
WARNINGS_ENABLE

/*!
 * \ingroup misc
 * \brief The Notification widget is a QSystemTrayIcon which displays
 * a "pop-up" notification in the user's main desktop.
 */
class Notification : public QSystemTrayIcon
{
    Q_OBJECT

public:
    //! Constructor.
    explicit Notification(QSystemTrayIcon *parent = nullptr);

public slots:
    //! Displays a string if the user has "Show desktop notifications" enabled,
    //! after removing HTML tags.
    void displayNotification(QString message);
};

#endif // NOTIFICATION_H
