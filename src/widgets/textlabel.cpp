#include "textlabel.h"

TextLabel::TextLabel(QWidget *parent) : QLabel(parent), _elide(Qt::ElideNone)
{
}

TextLabel::~TextLabel()
{
}
Qt::TextElideMode TextLabel::elide() const
{
    return _elide;
}

void TextLabel::setElide(const Qt::TextElideMode &elide)
{
    _elide = elide;
    emit elideChanged(_elide);
}

QString TextLabel::text()
{
    return _fullText;
}

QSize TextLabel::sizeHint() const
{
    QFontMetrics metrics(this->font());
    return metrics.size(Qt::TextSingleLine, _fullText);
}

void TextLabel::setText(const QString &text)
{
    _fullText = text;
    QLabel::setText(elideText(_fullText));
    setToolTip(_fullText);
}

void TextLabel::clear()
{
    _fullText.clear();
    QLabel::clear();
}

void TextLabel::resizeEvent(QResizeEvent *event)
{
    QLabel::setText(elideText(_fullText));

    if(event)
        event->ignore();
}

void TextLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->buttons() ^ Qt::LeftButton)
        emit clicked();
    event->ignore();
}

void TextLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
        emit doubleClicked();
    event->ignore();
}

QString TextLabel::elideText(const QString &text)
{
    QFontMetrics metrics(this->font());
    return metrics.elidedText(text, _elide, this->width());
}
