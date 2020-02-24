#ifndef OKLABEL_H
#define OKLABEL_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QLabel>
WARNINGS_ENABLE

/*!
 * \ingroup lib-widgets
 * \brief The OkLabel widget is a QLabel which displays success,
 * error, or nothing.
 *
 * In English, this is displayed by a green check, red cross, or blank;
 * it would be easy to alter this for other languages.
 */
class OkLabel : public QLabel
{
    Q_OBJECT

    //! Status to display.  We use int to avoid dealing with QMetaType,
    //! but the only meaningful values are listed in \ref Status.
    Q_PROPERTY(int status READ status WRITE setStatus DESIGNABLE true)

public:
    //! Constructor.
    explicit OkLabel(QWidget *parent = nullptr);

    //! What state should we display?
    enum Status
    {
        Unset = 0,
        Ok    = 1,
        Error = 2
    };

    //! Get the status.
    int status() const;
    //! Set the status.
    void setStatus(int status);

    //! Get a rich text representation of a status.
    static const QString getRichText(int status);

private:
    int _status;
};

#endif // OKLABEL_H
