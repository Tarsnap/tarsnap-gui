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
    void setDisabled();
    bool isDisabled();

signals:
    void requestDelete();
    void requestInspect();
    void requestRestore();
    void requestGoToJob();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void update();

private:
    Ui::ArchiveListWidgetItem _ui;
    QWidget    *_widget;
    ArchivePtr _archive;

    void setArchive(ArchivePtr archive);
};

#endif // ARCHIVELISTWIDGETITEM_H
