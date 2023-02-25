#include "TTabWidget.h"

WARNINGS_DISABLE
#include <QLabel>
#include <QPixmap>
#include <QTabBar>
#include <QTabWidget>

#include "ui_TTabWidget.h"
WARNINGS_ENABLE

class QResizeEvent;

#define RIGHT_PADDING 3

TTabWidget::TTabWidget(QWidget *parent)
    : QTabWidget(parent),
      _ui(new Ui::TTabWidget),
      _largeLogo(nullptr),
      _smallLogo(nullptr),
      _cornerLabel(new QLabel)
{
    _ui->setupUi(this);
    setCornerWidget(_cornerLabel);
}

TTabWidget::~TTabWidget()
{
    delete _largeLogo;
    delete _smallLogo;
    delete _ui;
}

void TTabWidget::setLargeLogoFilename(const QString &largeLogoFilename)
{
    // Save filename and load logo.
    _largeLogoFilename = largeLogoFilename;
    _largeLogo         = new QPixmap(_largeLogoFilename);
    recalculateWidth();
}

void TTabWidget::setSmallLogoFilename(const QString &smallLogoFilename)
{
    // Save filename and load logo.
    _smallLogoFilename = smallLogoFilename;
    _smallLogo         = new QPixmap(_smallLogoFilename);
    recalculateWidth();
}

void TTabWidget::resizeEvent(QResizeEvent *event)
{
    QTabWidget::resizeEvent(event);
    recalculateWidth();
}

void TTabWidget::tabInserted(int index)
{
    Q_UNUSED(index);
    recalculateWidth();
}

void TTabWidget::tabRemoved(int index)
{
    Q_UNUSED(index);
    recalculateWidth();
}

void TTabWidget::recalculateWidth()
{
    // Bail if we're missing either logo.
    if((!_largeLogo) || (!_smallLogo))
        return;

    // How much width is available?
    const int remainingWidth = width() - tabBar()->width() - RIGHT_PADDING;

    // Pick which image (if any) to use.
    if(remainingWidth > _largeLogo->width())
        _cornerLabel->setPixmap(*_largeLogo);
    else if(remainingWidth > _smallLogo->width())
        _cornerLabel->setPixmap(*_smallLogo);
    else
        _cornerLabel->clear();
    _cornerLabel->setContentsMargins(0, 0, RIGHT_PADDING, 0);
}
