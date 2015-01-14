#ifndef BACKUPLISTWIDGET_H
#define BACKUPLISTWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QUrl>
#include <QDir>

class BackupListWidget : public QListWidget
{
public:
    BackupListWidget(QWidget *parent);
    ~BackupListWidget();

    void addUrl(QUrl url);

protected:
    void dragMoveEvent(QDragMoveEvent* event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent * event);

private:
    qint64 getDirSize(QDir dir);
    qint64 getDirCount(QDir dir);
private:
    QWidget *_parent;
};

#endif // BACKUPLISTWIDGET_H
