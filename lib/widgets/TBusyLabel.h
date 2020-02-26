#ifndef TBUSYLABEL_H
#define TBUSYLABEL_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QLabel>
WARNINGS_ENABLE

/*!
 * \ingroup lib-widgets
 * \brief The TBusyLabel is a QLabel which displays a spinning "waiting" icon,
 * and adds the \ref clicked signal.
 */
class TBusyLabel : public QLabel
{
    Q_OBJECT

public:
    //! Constructor.
    explicit TBusyLabel(QWidget *parent = nullptr);
    ~TBusyLabel();

public slots:
    //! Starts the animation.
    //! \param active show the "waiting" icon.
    void animate(bool active);

signals:
    //! Emitted when this widget was clicked.
    void clicked();

protected:
    //! Used for detecting whether to emit \ref clicked.
    void mouseReleaseEvent(QMouseEvent *event);

private:
    QMovie *_animation;
};

#endif // !TBUSYLABEL_H
