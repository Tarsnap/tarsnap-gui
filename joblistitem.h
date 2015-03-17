#ifndef JOBLISTITEM_H
#define JOBLISTITEM_H

#include "ui_jobitemwidget.h"
#include "ui_jobwidget.h"

#include <QObject>
#include <QListWidgetItem>

#define EXPANDED_HEIGHT 300

namespace Ui {
class JobItemWidget;
class JobWidget;
}

class JobListItem : public QObject, public QListWidgetItem
{
    Q_OBJECT
public:
    explicit JobListItem(QObject *parent = 0);
    ~JobListItem();

    QWidget *widget();

signals:

public slots:
    void displayDrawer(bool display);
    void toggleDrawer();

protected:
//    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::JobItemWidget   _itemUi;
    Ui::JobWidget       _jobUi;
    QWidget             _widget;
    int                 _expandedHeight;
    int                 _defaultHeight;
};

#endif // JOBLISTITEM_H
