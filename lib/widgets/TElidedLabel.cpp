#include "TElidedLabel.h"

WARNINGS_DISABLE
#include <QFontMetrics>
#include <QResizeEvent>
WARNINGS_ENABLE

TElidedLabel::TElidedLabel(QWidget *parent)
    : QLabel(parent), _elide(Qt::ElideNone)
{
}

Qt::TextElideMode TElidedLabel::elide() const
{
    return _elide;
}

void TElidedLabel::setElide(const Qt::TextElideMode &elide)
{
    _elide = elide;
    emit elideChanged(_elide);
}

QString TElidedLabel::text() const
{
    return _fullText;
}

QSize TElidedLabel::sizeHint() const
{
    QFontMetrics metrics(this->font());
    return metrics.size(Qt::TextSingleLine, _fullText);
}

void TElidedLabel::setText(const QString &text)
{
    _fullText = text;
    QLabel::setText(elideText(_fullText));
    setToolTip(_fullText);
}

void TElidedLabel::clear()
{
    _fullText.clear();
    QLabel::clear();
    setToolTip("");
}

void TElidedLabel::resizeEvent(QResizeEvent *event)
{
    QLabel::setText(elideText(_fullText));
    event->accept();
}

QString TElidedLabel::elideText(const QString &text) const
{
    QFontMetrics metrics(this->font());
    return metrics.elidedText(text, _elide, this->width());
}

void TElidedLabel::messageNormal(const QString &text)
{
    setText(text);
    setStyleSheet("");
}

void TElidedLabel::messageError(const QString &text)
{
    setText(text);
    setStyleSheet("color: darkred;");
}
