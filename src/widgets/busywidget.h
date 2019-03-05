#ifndef BUSYWIDGET_H
#define BUSYWIDGET_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QLabel>
#include <QMovie>
WARNINGS_ENABLE

/*!
 * \ingroup widgets-general
 * \brief The BusyWidget is a QLabel which displays a spinning "waiting" icon,
 * and adds the \ref clicked signal.
 */
class BusyWidget : public QLabel
{
    Q_OBJECT

public:
    //! Constructor.
    explicit BusyWidget(QWidget *parent = nullptr);
    ~BusyWidget();

public slots:
    //! Starts the animation.
    void animate();
    //! Stops the animation.
    void stop();

signals:
    //! Emitted when this widget was clicked.
    void clicked();

protected:
    //! Used for detecting whether to emit \ref clicked.
    void mouseReleaseEvent(QMouseEvent *event);

private:
    QMovie _animation;
};

#endif // BUSYWIDGET_H
