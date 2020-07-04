#include "elidedannotatedlabel.h"

WARNINGS_DISABLE
#include <QFontMetrics>
#include <QLabel>
#include <QResizeEvent>
WARNINGS_ENABLE

ElidedAnnotatedLabel::ElidedAnnotatedLabel(QWidget *parent)
    : TElidedLabel(parent)
{
}

void ElidedAnnotatedLabel::setAnnotatedText(const QVector<QString> &texts,
                                            const QVector<QString> &annotations)
{
    _texts       = texts;
    _annotations = annotations;
    Q_ASSERT(2 * (_texts.size()) == _annotations.size());

    // Construct plain text
    _fullText = "";
    for(int p = 0; p < texts.size(); p++)
        _fullText += texts.at(p);

    QLabel::setText(elideText());
    setToolTip(_fullText);
}

void ElidedAnnotatedLabel::clear()
{
    TElidedLabel::clear();
    _texts.clear();
    _annotations.clear();
}

void ElidedAnnotatedLabel::resizeEvent(QResizeEvent *event)
{
    QLabel::setText(elideText());
    event->accept();
}

QString ElidedAnnotatedLabel::elideText()
{
    QFontMetrics metrics(this->font());
    QString      elided       = "";
    bool         shouldElide  = false;
    QString      elided_plain = TElidedLabel::elideText(_fullText);
    int          remaining    = elided_plain.length();

    // Should we elide?
    if(elided_plain.endsWith("…"))
    {
        shouldElide = true;
        // Take off an extra character to reserve space for the elipsis
        remaining -= 1;
        // Special case for a tiny label
        if(remaining == 0)
            elided = "…";
    }

    // Construct HTML string to match the plaintext string
    for(int p = 0; p < _texts.size(); p++)
    {
        if(remaining == 0)
            break;

        // Append the pre-annotation
        elided += _annotations[2 * p];

        if(remaining >= _texts[p].size())
        {
            // Append entire string
            elided += _texts[p];
            remaining -= _texts[p].size();
        }
        else
        {
            // Append only a few characters
            const QString to_append = _texts[p].left(remaining);
            elided += to_append;
            remaining -= to_append.size();
        }

        // If we're done, add the elipsis
        if(remaining == 0 && shouldElide)
            elided += "…";

        // Append the post-annotation
        elided += _annotations[2 * p + 1];
    }

    return elided;
}
