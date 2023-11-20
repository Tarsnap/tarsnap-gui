#include "TAsideLabel.h"

WARNINGS_DISABLE
#include <QFont>
WARNINGS_ENABLE

#define SHEET_ASIDE                                                            \
    "margin-left: 0.125em;"                                                    \
    "border-width: 0 0 0 0.1em;"                                               \
    "border-style: solid;"                                                     \
    "border-color: palette(mid);"                                              \
    "padding-left: 0.25em;"

TAsideLabel::TAsideLabel(QWidget *parent) : QLabel(parent)
{
    // Make it italic.
    QFont font = this->font();
    font.setStyle(QFont::StyleItalic);
    setFont(font);

    // Add the border and padding.
    setStyleSheet(SHEET_ASIDE);

    // Word wrap.
    setWordWrap(true);
}
