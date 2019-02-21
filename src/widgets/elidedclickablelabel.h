#ifndef ELIDEDCLICKABLELABEL_H
#define ELIDEDCLICKABLELABEL_H

#include "elidedlabel.h"

/*!
 * \ingroup widgets-general
 * \brief The ElidedClickableLabel widget is a QLabel which automatically elides
 * (`...`) long text, and adds the \ref clicked signal.
 */
class ElidedClickableLabel : public ElidedLabel
{
    Q_OBJECT

public:
    //! Constructor.
    explicit ElidedClickableLabel(QWidget *parent = nullptr);
    ~ElidedClickableLabel();

signals:
    //! Emitted when this widget was clicked.
    void clicked();

private:
    //! Used for detecting whether to emit \ref clicked.
    void mouseReleaseEvent(QMouseEvent *event);
};

#endif // ELIDEDCLICKABLELABEL_H
