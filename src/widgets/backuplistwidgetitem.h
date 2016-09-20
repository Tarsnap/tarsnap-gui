#ifndef BACKUPLISTWIDGETITEM_H
#define BACKUPLISTWIDGETITEM_H

#include "ui_backuplistwidgetitem.h"

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

    quint64 count() const;

    quint64 size() const;

public slots:
    void browseUrl();

signals:
    void requestDelete();
    void requestUpdate();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void updateDirDetail(quint64 size, quint64 count);

private:
    Ui::BackupListWidgetItem _ui;
    QWidget             *_widget;
    QUrl                 _url;
    quint64              _count;
    quint64              _size;

    void setUrl(const QUrl &url);
};

#endif // BACKUPLISTITEM_H
