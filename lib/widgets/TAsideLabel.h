#ifndef TASIDELABEL_H
#define TASIDELABEL_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QLabel>
#include <QObject>
WARNINGS_ENABLE

/* Forward declaration(s). */
class QWidget;

/*!
 * \ingroup lib-widgets
 * \brief The TAsideLabel widget is a QLabel which is similar to \<aside\>.
 */
class TAsideLabel : public QLabel
{
    Q_OBJECT

public:
    //! Constructor.
    explicit TAsideLabel(QWidget *parent = nullptr);
};

#endif // TASIDELABEL_H
