#ifndef BACKUPLISTITEM_H
#define BACKUPLISTITEM_H

#include "ui_backupitemwidget.h"

#include <QListWidgetItem>
#include <QObject>
#include <QUrl>

class BackupListItem : public QObject, public QListWidgetItem
{
    Q_OBJECT

public:
    explicit BackupListItem(QUrl url);
    ~BackupListItem();

    QWidget *widget();

    QUrl url() const;
    void setUrl(const QUrl &url);

    quint64 count() const;
    void setCount(const quint64 &count);

    quint64 size() const;
    void setSize(const quint64 &size);

public slots:
    void browseUrl();
    void updateDirDetail(quint64 size, quint64 count);

signals:
    void requestDelete();
    void requestUpdate();

private:
    Ui::BackupItemWidget _ui;
    QWidget             *_widget;
    QUrl                 _url;
    quint64              _count;
    quint64              _size;
    bool                 _useIECPrefixes;
};

#endif // BACKUPLISTITEM_H
