#ifndef ARCHIVELISTITEM_H
#define ARCHIVELISTITEM_H

#include "jobmanager.h"
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
    explicit ArchiveListItem(QSharedPointer<Archive> archive, QObject *parent = 0);
    ~ArchiveListItem();

    QWidget *widget();

    QSharedPointer<Archive> archive() const;
    void setArchive(QSharedPointer<Archive> archive);

public slots:

signals:
    void requestDelete();
    void requestInspect();
    void requestRestore();

private:
    Ui::ArchiveItemWidget    _ui;
    QWidget                  _widget;
    QSharedPointer<Archive>  _archive;
};

#endif // ARCHIVELISTITEM_H
