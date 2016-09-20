#ifndef BACKUPLISTWIDGETITEM_H
#define BACKUPLISTWIDGETITEM_H

#include "ui_backupitemwidget.h"

#include <QListWidgetItem>
#include <QObject>
#include <QUrl>

class BackupListWidgetItem : public QObject, public QListWidgetItem
{
    Q_OBJECT

public:
    explicit BackupListWidgetItem(QUrl url);
    ~BackupListWidgetItem();

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

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::BackupItemWidget _ui;
    QWidget             *_widget;
    QUrl                 _url;
    quint64              _count;
    quint64              _size;
};

#endif // BACKUPLISTITEM_H
