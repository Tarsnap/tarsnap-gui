#include "elidedlabel.h"

#include <QMouseEvent>

ElidedLabel::ElidedLabel(QWidget *parent) : QLabel(parent), _elide(Qt::ElideNone)
{
}

ElidedLabel::~ElidedLabel()
{
}

Qt::TextElideMode ElidedLabel::elide() const
{
    return _elide;
}

void ElidedLabel::setElide(const Qt::TextElideMode &elide)
{
    _elide = elide;
    emit elideChanged(_elide);
}

QString ElidedLabel::text()
{
    return _fullText;
}

QSize ElidedLabel::sizeHint() const
{
    QFontMetrics metrics(this->font());
    return metrics.size(Qt::TextSingleLine, _fullText);
}

void ElidedLabel::setText(const QString &text)
{
    _fullText = text;
    QLabel::setText(elideText(_fullText));
    setToolTip(_fullText);
}

void ElidedLabel::clear()
{
    _fullText.clear();
    QLabel::clear();
}

void ElidedLabel::resizeEvent(QResizeEvent *event)
{
    QLabel::setText(elideText(_fullText));
    event->accept();
}

void ElidedLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        emit clicked();
    event->accept();
}

QString ElidedLabel::elideText(const QString &text)
{
    QFontMetrics metrics(this->font());
    return metrics.elidedText(text, _elide, this->width());
}
