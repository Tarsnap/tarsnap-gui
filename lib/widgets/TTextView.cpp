#include "TTextView.h"

WARNINGS_DISABLE
#include <QApplication>
#include <QBrush>
#include <QDateTime>
#include <QPalette>
#include <QTextBlockFormat>
#include <QTextDocument>
#include <Qt>
WARNINGS_ENABLE

#include "LogEntry.h"

TTextView::TTextView(QWidget *parent)
    : QPlainTextEdit(parent), _cursor(document())
{
    setReadOnly(true);

    // Store these values for easier re-use.
    _baseColor = qApp->palette().base().color();
    _altColor  = qApp->palette().alternateBase().color();
}

void TTextView::appendLog(const LogEntry &log)
{
    appendAlternatingText(
        QString("[%1] %2")
            .arg(log.timestamp.toString(Qt::DefaultLocaleShortDate))
            .arg(log.message));
}

void TTextView::appendAlternatingText(const QString &text)
{
    // Start a new block.
    if(!document()->isEmpty())
        _cursor.insertBlock();

    // Alternate the background color.
    QTextBlockFormat fmt;
    if(_cursor.blockFormat().background().color() == _baseColor)
        fmt.setBackground(QBrush(_altColor));
    else
        fmt.setBackground(QBrush(_baseColor));
    _cursor.mergeBlockFormat(fmt);

    // Append the text.
    _cursor.insertText(text);

    // Scroll to display the recently-added text.
    ensureCursorVisible();
}
