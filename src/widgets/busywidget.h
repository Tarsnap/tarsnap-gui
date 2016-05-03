#ifndef BUSYWIDGET_H
#define BUSYWIDGET_H

#include <QLabel>
#include <QMouseEvent>
#include <QMovie>

class BusyWidget : public QLabel
{
    Q_OBJECT

public:
    explicit BusyWidget(QWidget *parent = NULL);
    ~BusyWidget();

public slots:
    void animate();
    void stop();

signals:
    void clicked();

protected:
    void mouseReleaseEvent(QMouseEvent *event);

private:
    QMovie _animation;
};

#endif // BUSYWIDGET_H
