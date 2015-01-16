#ifndef BACKUPLISTWIDGET_H
#define BACKUPLISTWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QUrl>
#include <QAction>

class BackupListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit BackupListWidget(QWidget *parent = 0);
    ~BackupListWidget();

public slots:
    void removeItem();
    void removeSelectedItems();

protected:
    void dragMoveEvent(QDragMoveEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private:
    QAction _actionRemoveItems;

    void addItemWithUrl(QUrl url);
};

#endif // BACKUPLISTWIDGET_H
