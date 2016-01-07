#ifndef BACKUPLISTWIDGET_H
#define BACKUPLISTWIDGET_H

#include <QListWidget>
#include <QUrl>
#include <QWidget>

class BackupListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit BackupListWidget(QWidget *parent = 0);
    ~BackupListWidget();

public slots:
    void addItemWithUrl(QUrl url);
    void addItemsWithUrls(QList<QUrl> urls);
    void removeItems();
    void recomputeListTotals();

signals:
    void itemTotals(quint64 count, quint64 size);

protected:
    void dragMoveEvent(QDragMoveEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private:
};

#endif // BACKUPLISTWIDGET_H
