#ifndef BACKUPLISTITEM_H
#define BACKUPLISTITEM_H

#include "ui_backupitemwidget.h"

#include <QObject>
#include <QListWidgetItem>
#include <QUrl>

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

    qint64 count() const;
    void setCount(const qint64 &count);

    qint64 size() const;
    void setSize(const qint64 &size);

signals:
    void requestDelete();
    void requestUpdate();

public slots:
    void browseUrl();
    void updateDirDetail(qint64 size, qint64 count);

private:
    Ui::BackupItemWidget _ui;
    QWidget              _widget;
    QUrl                 _url;
    qint64               _count;
    qint64               _size;
};

#endif // BACKUPLISTITEM_H
