#ifndef BROWSELISTWIDGET_H
#define BROWSELISTWIDGET_H

#include "jobmanager.h"

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
    void removeItem();
    void inspectItem();
    void restoreItem();
    void removeSelectedItems();

signals:
    void getArchivesList();
    void deleteArchive(ArchivePtr archive);
    void inspectArchive(ArchivePtr archive);

protected:
    void keyReleaseEvent(QKeyEvent *event);

private:
//    QList<ArchivePtr> _archives;
};

#endif // BROWSELISTWIDGET_H
