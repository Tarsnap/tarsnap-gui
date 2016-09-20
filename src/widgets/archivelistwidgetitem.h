#ifndef ARCHIVELISTWIDGETITEM_H
#define ARCHIVELISTWIDGETITEM_H

#include "taskmanager.h"
#include "ui_archivelistwidgetitem.h"

#include <QListWidgetItem>
#include <QObject>

class ArchiveListWidgetItem : public QObject, public QListWidgetItem
{
    Q_OBJECT

public:
    explicit ArchiveListWidgetItem(ArchivePtr archive);
    ~ArchiveListWidgetItem();

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
    Ui::ArchiveListWidgetItem _ui;
    QWidget    *_widget;
    ArchivePtr _archive;
};

#endif // ARCHIVELISTWIDGETITEM_H
