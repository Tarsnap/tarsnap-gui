#ifndef ELIDEDANNOTATEDLABEL_H
#define ELIDEDANNOTATEDLABEL_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
#include <QString>
#include <QVector>
WARNINGS_ENABLE

#include "TElidedLabel.h"

/* Forward declaration(s). */
class QResizeEvent;
class QWidget;

/*!
 * \ingroup widgets-general
 * \brief The ElidedAnnotatedLabel widget is a QLabel which automatically
 * elides (`...`) long text without being confused by HTML tags.
 */
class ElidedAnnotatedLabel : public TElidedLabel
{
    Q_OBJECT

#ifdef QT_TESTLIB_LIB
    friend class TestSmallWidgets;
#endif

public:
    //! Constructor.
    explicit ElidedAnnotatedLabel(QWidget *parent = nullptr);

public slots:
    //! Sets the text portions and HTML annotations for this widget to display
    //! (may be elided).  The annotations must be twice as long as the texts,
    //! as each portion has starting and ending tags.
    void setAnnotatedText(const QVector<QString> &texts,
                          const QVector<QString> &annotations);
    //! Clears all contents, including the full text.
    void clear();

protected:
    //! Recalculates the amount of elided text to display.
    void resizeEvent(QResizeEvent *event) override;

private:
    // Returns an elided version of the string (if necessary), or the full
    // string (if not).  In both cases, the annotations are prepended and
    // appended as appropriate.
    QString elideText();

    QVector<QString> _texts;
    QVector<QString> _annotations;
};

#endif // ELIDEDANNOTATEDLABEL_H
