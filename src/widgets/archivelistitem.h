#ifndef ARCHIVELISTITEM_H
#define ARCHIVELISTITEM_H

#include "taskmanager.h"
#include "ui_archiveitemwidget.h"

#include <QListWidgetItem>
#include <QObject>

class ArchiveListItem : public QObject, public QListWidgetItem
{
    Q_OBJECT

public:
    explicit ArchiveListItem(ArchivePtr archive);
    ~ArchiveListItem();

    QWidget *widget();

    ArchivePtr archive() const;
    void setArchive(ArchivePtr archive);
    void setDisabled();
    bool isDisabled();

public slots:
    void update();

signals:
    void requestDelete();
    void requestInspect();
    void requestRestore();
    void requestGoToJob();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::ArchiveItemWidget _ui;
    QWidget              *_widget;
    ArchivePtr            _archive;
};

#endif // ARCHIVELISTITEM_H
