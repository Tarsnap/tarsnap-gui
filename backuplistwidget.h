#ifndef BACKUPLISTWIDGET_H
#define BACKUPLISTWIDGET_H

#include <QWidget>
#include <QListWidget>

class BackupListWidget : public QListWidget
{
public:
    BackupListWidget(QWidget *parent);
    ~BackupListWidget();

protected:
    void dragMoveEvent(QDragMoveEvent* event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent * event);

private:
    QWidget *_parent;
};

#endif // BACKUPLISTWIDGET_H
