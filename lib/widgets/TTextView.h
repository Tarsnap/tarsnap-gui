#ifndef TTEXTVIEW_H
#define TTEXTVIEW_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QColor>
#include <QObject>
#include <QPlainTextEdit>
#include <QTextCursor>
WARNINGS_ENABLE

#include "LogEntry.h"

/*!
 * \ingroup lib-widgets
 * \brief The TTextView widget is a QPlainTextEdit which displays log messages,
 * with alternating background colors.
 */
class TTextView : public QPlainTextEdit
{
    Q_OBJECT

public:
    //! Constructor.
    TTextView(QWidget *parent = nullptr);

    //! Append a log message to the display.
    void appendLog(const LogEntry &log);

private:
    // Convenience variables (to avoid regenerating them all the time).
    QTextCursor _cursor;
    QColor      _baseColor;
    QColor      _altColor;

    // Append text, with alternating background colors.
    void appendAlternatingText(const QString &text);
};

#endif // !TTEXTVIEW_H
