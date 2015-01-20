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
    void addItemWithUrl(QUrl url);
    void addItemsWithUrls(QList<QUrl> urls);
    void removeItem();
    void removeSelectedItems();
    void recomputeListTotals();
    void clear();

signals:
    void itemTotals(qint64 count, qint64 size);

protected:
    void dragMoveEvent(QDragMoveEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private:
    QAction _actionRemoveItems;
};

#endif // BACKUPLISTWIDGET_H
