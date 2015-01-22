#ifndef ARCHIVELISTITEM_H
#define ARCHIVELISTITEM_H

#include "ui_archiveitemwidget.h"

#include <QObject>
#include <QListWidgetItem>

namespace Ui {
class ArchiveItemWidget;
}

class ArchiveListItem : public QObject, public QListWidgetItem
{
    Q_OBJECT

public:
    explicit ArchiveListItem(QObject *parent = 0);
    ~ArchiveListItem();

private:
    Ui::ArchiveItemWidget    _ui;
    QWidget                 *_widget;
};

#endif // ARCHIVELISTITEM_H
