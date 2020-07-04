#ifndef TELIDEDLABEL_H
#define TELIDEDLABEL_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QLabel>
#include <QObject>
#include <QSize>
#include <QString>
#include <Qt>
WARNINGS_ENABLE

/* Forward declaration(s). */
class QResizeEvent;
class QWidget;

/*!
 * \ingroup lib-widgets
 * \brief The TElidedLabel widget is a QLabel which automatically elides
 * (`...`) long text.
 */
class TElidedLabel : public QLabel
{
    Q_OBJECT

    //! Store the potentially-elided text.
    Q_PROPERTY(QString text READ text WRITE setText DESIGNABLE true)
    //! Indicates how the text should be elided.
    Q_PROPERTY(Qt::TextElideMode elide READ elide WRITE setElide NOTIFY
                   elideChanged DESIGNABLE true)

public:
    //! Constructor.
    explicit TElidedLabel(QWidget *parent = nullptr);

    //! Returns the elided mode.
    Qt::TextElideMode elide() const;
    //! Sets the elided mode.
    void setElide(const Qt::TextElideMode &elide);

    //! Returns the full text.
    QString text() const;
    //! Returns an elided version of the string (if necessary), or
    //! the original string (if not).
    QString elideText(const QString &text) const;

    //! Returns the recommended size for the widget based on the full text.
    QSize sizeHint() const override;

    //! Normal status message.
    void messageNormal(const QString &text);

    //! Error status message.
    void messageError(const QString &text);

public slots:
    //! Sets the full text for this TElidedLabel to display (may be elided).
    void setText(const QString &text);
    //! Clears all contents, including the full text.
    void clear();

signals:
    //! Emitted when the elide mode was changed.
    void elideChanged(Qt::TextElideMode elide);

protected:
    //! Recalculates the amount of elided text to display.
    void resizeEvent(QResizeEvent *event) override;

    //! How should the text be elided?
    Qt::TextElideMode _elide;

    //! Non-elided text.
    QString _fullText;
};

#endif // TELIDEDLABEL_H
