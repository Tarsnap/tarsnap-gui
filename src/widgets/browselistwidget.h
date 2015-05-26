#ifndef BROWSELISTWIDGET_H
#define BROWSELISTWIDGET_H

#include "persistentmodel/archive.h"

#include <QListWidget>
#include <QList>

class BrowseListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit BrowseListWidget(QWidget *parent = 0);
    ~BrowseListWidget();

public slots:
    void addArchives(QList<ArchivePtr> archives);
    void setSelectedArchive(ArchivePtr archive);

private slots:
    void removeItems();
    void inspectItem();
    void restoreItem();

signals:
    void getArchiveList();
    void deleteArchives(QList<ArchivePtr> archives);
    void inspectArchive(ArchivePtr archive);
    void restoreArchive(ArchivePtr archive, ArchiveRestoreOptions options);

protected:
    void keyReleaseEvent(QKeyEvent *event);

private:
//    QList<ArchivePtr> _archives;
};

#endif // BROWSELISTWIDGET_H
