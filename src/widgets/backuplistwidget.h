#ifndef BACKUPLISTWIDGET_H
#define BACKUPLISTWIDGET_H

#include <QListWidget>
#include <QUrl>
#include <QWidget>

class BackupListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit BackupListWidget(QWidget *parent = nullptr);
    ~BackupListWidget();

public slots:
    void        addItemWithUrl(QUrl url);
    void        addItemsWithUrls(QList<QUrl> urls);
    QList<QUrl> itemUrls();
    void        removeItems();
    void        recomputeListTotals();
    void        setItemsWithUrls(QList<QUrl> urls);

signals:
    void itemTotals(quint64 count, quint64 size);
    void itemWithUrlAdded(QUrl url);

protected:
    void dragMoveEvent(QDragMoveEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private:
};

#endif // BACKUPLISTWIDGET_H
