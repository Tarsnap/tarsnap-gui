#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QObject>
#include <QSystemTrayIcon>

class Notification : public QSystemTrayIcon
{
    Q_OBJECT
public:
    explicit Notification(QSystemTrayIcon *parent = nullptr);

public slots:
    void displayNotification(QString message);

signals:
};

#endif // NOTIFICATION_H
