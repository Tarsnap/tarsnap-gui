#ifndef BACKUPLISTWIDGET_H
#define BACKUPLISTWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QUrl>

class BackupListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit BackupListWidget(QWidget *parent = 0);
    ~BackupListWidget();

public slots:
    void removeItem();

protected:
    void dragMoveEvent(QDragMoveEvent* event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent * event);

private:
    void addItemWithUrl(QUrl url);
};

#endif // BACKUPLISTWIDGET_H
