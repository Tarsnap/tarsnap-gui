#ifndef ELIDEDCLICKABLELABEL_H
#define ELIDEDCLICKABLELABEL_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
#include <QString>
WARNINGS_ENABLE

#include "TElidedLabel.h"

/* Forward declaration(s). */
class QMouseEvent;
class QWidget;

/*!
 * \ingroup widgets-general
 * \brief The ElidedClickableLabel widget is a QLabel which automatically
 * elides (`...`) long text, and adds the \ref clicked signal.
 */
class ElidedClickableLabel : public TElidedLabel
{
    Q_OBJECT

public:
    //! Constructor.
    explicit ElidedClickableLabel(QWidget *parent = nullptr);

signals:
    //! Emitted when this widget was clicked.
    void clicked();

private:
    //! Used for detecting whether to emit \ref clicked.
    void mouseReleaseEvent(QMouseEvent *event) override;
};

#endif // ELIDEDCLICKABLELABEL_H
