#ifndef JOBWIDGET_H
#define JOBWIDGET_H

#include "ui_jobwidget.h"

#include <QWidget>

namespace Ui {
class JobWidget;
}

class JobWidget : public QWidget
{
    Q_OBJECT
public:
    explicit JobWidget(QWidget *parent = 0);
    ~JobWidget();

signals:

public slots:

protected:
    void focusInEvent(QFocusEvent * event);
    void ​focusOutEvent(QFocusEvent * event);
    void mouseDoubleClickEvent(QMouseEvent *);

private slots:
    void on_browseButton_clicked();

    void on_backButton_clicked();

private:
    Ui::JobWidget   *_ui;
};

#endif // JOBWIDGET_H
