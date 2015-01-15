#ifndef BACKUPLISTITEM_H
#define BACKUPLISTITEM_H

#include <QObject>
#include <QListWidgetItem>
#include <QUrl>
#include <QDir>

#include "ui_backupitemwidget.h"

namespace Ui {
class BackupItemWidget;
}

class BackupListItem : public QObject, public QListWidgetItem
{
    Q_OBJECT

public:
    BackupListItem(QUrl url);
    ~BackupListItem();

    QWidget *widget();

    QUrl url() const;
    void setUrl(const QUrl &url);

    void cleanup();

signals:
    void requestDelete();

public slots:
    void browseUrl();

private:
    Ui::BackupItemWidget _ui;
    QWidget              *_widget;
    QUrl                 _url;

    qint64 getDirSize(QDir dir);
    qint64 getDirCount(QDir dir);
};

#endif // BACKUPLISTITEM_H
