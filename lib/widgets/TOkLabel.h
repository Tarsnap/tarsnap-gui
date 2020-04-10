#ifndef TOKLABEL_H
#define TOKLABEL_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QLabel>
#include <QObject>
#include <QString>
WARNINGS_ENABLE

/* Forward declaration(s). */
class QEvent;
class QWidget;

/*!
 * \ingroup lib-widgets
 * \brief The TOkLabel widget is a QLabel which displays success,
 * error, or nothing.
 *
 * In English, this is displayed by a green check, red cross, or blank;
 * it would be easy to alter this for other languages.
 */
class TOkLabel : public QLabel
{
    Q_OBJECT

    //! Status to display.  We use int to avoid dealing with QMetaType,
    //! but the only meaningful values are listed in \ref Status.
    Q_PROPERTY(int status READ status WRITE setStatus DESIGNABLE true)

public:
    //! Constructor.
    explicit TOkLabel(QWidget *parent = nullptr);

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

protected:
    //! Handle translation change of language.
    void changeEvent(QEvent *event) override;

private:
    int _status;
};

#endif // TOKLABEL_H
