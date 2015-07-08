#ifndef ARCHIVELISTITEM_H
#define ARCHIVELISTITEM_H

#include "taskmanager.h"
#include "ui_archiveitemwidget.h"

#include <QObject>
#include <QListWidgetItem>
#include <QSharedPointer>

namespace Ui {
class ArchiveItemWidget;
}

class ArchiveListItem : public QObject, public QListWidgetItem
{
    Q_OBJECT

public:
    explicit ArchiveListItem(ArchivePtr archive);
    ~ArchiveListItem();

    QWidget *widget();

    ArchivePtr archive() const;
    void setArchive(ArchivePtr archive);

public slots:
    void update();

signals:
    void requestDelete();
    void requestInspect();
    void requestRestore();
    void requestGoToJob();

private:
    Ui::ArchiveItemWidget    _ui;
    QWidget                  _widget;
    ArchivePtr               _archive;
    bool                     _useSIPrefixes;
};

#endif // ARCHIVELISTITEM_H
