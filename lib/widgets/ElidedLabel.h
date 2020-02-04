#ifndef ELIDEDLABEL_H
#define ELIDEDLABEL_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QLabel>
WARNINGS_ENABLE

/*!
 * \ingroup lib-widgets
 * \brief The ElidedLabel widget is a QLabel which automatically elides
 * (`...`) long text.
 */
class ElidedLabel : public QLabel
{
    Q_OBJECT

    //! Indicates how the text should be elided.
    Q_PROPERTY(Qt::TextElideMode elide READ elide WRITE setElide NOTIFY
                   elideChanged DESIGNABLE true)

#ifdef QT_TESTLIB_LIB
    friend class TestSmallWidgets;
#endif

public:
    //! Constructor.
    explicit ElidedLabel(QWidget *parent = nullptr);
    ~ElidedLabel();

    //! Returns the elided mode.
    Qt::TextElideMode elide() const;
    //! Sets the elided mode.
    void setElide(const Qt::TextElideMode &elide);

    //! Returns the full text.
    QString text() const;

    //! Returns the recommended size for the widget based on the full text.
    QSize sizeHint() const;

    //! Normal status message.
    void messageNormal(const QString &text);

    //! Error status message.
    void messageError(const QString &text);

public slots:
    //! Sets the full text for this ElidedLabel to display (may be elided).
    void setText(const QString &text);
    //! Clears all contents, including the full text.
    void clear();

signals:
    //! Emitted when the elide mode was changed.
    void elideChanged(Qt::TextElideMode elide);

protected:
    //! Recalculates the amount of elided text to display.
    void resizeEvent(QResizeEvent *event);

    //! How should the text be elided?
    Qt::TextElideMode _elide;

    //! Non-elided text.
    QString _fullText;

    //! Returns an elided version of the string (if necessary), or
    //! the original string (if not).
    QString elideText(const QString &text) const;
};

#endif // ELIDEDLABEL_H
