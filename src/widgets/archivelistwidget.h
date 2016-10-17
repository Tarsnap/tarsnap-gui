#ifndef ARCHIVELISTWIDGET_H
#define ARCHIVELISTWIDGET_H

#include "persistentmodel/archive.h"
#include "persistentmodel/job.h"

#include <QList>
#include <QListWidget>

class ArchiveListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit ArchiveListWidget(QWidget *parent = nullptr);
    ~ArchiveListWidget();

public slots:
    void setArchives(QList<ArchivePtr> archives);
    void setSelectedArchive(ArchivePtr archive);
    void disableArchives(QList<ArchivePtr> archives);
    void removeSelectedItems();
    void inspectSelectedItem();
    void restoreSelectedItem();
    void setFilter(QString regex);

signals:
    void deleteArchives(QList<ArchivePtr> archives);
    void inspectArchive(ArchivePtr archive);
    void restoreArchive(ArchivePtr archive, ArchiveRestoreOptions options);
    void displayJobDetails(QString jobRef);

protected:
    void keyPressEvent(QKeyEvent *event);

private slots:
    void removeItem();
    void inspectItem();
    void restoreItem();
    void goToJob();

private:
    QRegExp _filter;
};

#endif // ARCHIVELISTWIDGET_H
