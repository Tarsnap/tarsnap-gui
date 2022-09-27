#include "TTabWidget.h"

WARNINGS_DISABLE
#include <QPainter>
#include <QPixmap>
#include <QTabBar>
#include <QTabWidget>

#include "ui_TTabWidget.h"
WARNINGS_ENABLE

class QPaintEvent;
class QResizeEvent;

#define RIGHT_PADDING 3

TTabWidget::TTabWidget(QWidget *parent)
    : QTabWidget(parent),
      _ui(new Ui::TTabWidget),
      _needRecalculate(true),
      _image_x(0),
      _largeLogo(nullptr),
      _smallLogo(nullptr),
      _image(nullptr)
{
    _ui->setupUi(this);
}

TTabWidget::~TTabWidget()
{
    delete _largeLogo;
    delete _smallLogo;
    delete _ui;
}

void TTabWidget::setLargeLogoFilename(const QString &largeLogoFilename)
{
    // Check that we haven't used this before.
    Q_ASSERT(_largeLogo == nullptr);

    // Save filename and load logo.
    _largeLogoFilename = largeLogoFilename;
    _largeLogo         = new QPixmap(_largeLogoFilename);
    _needRecalculate   = true;
}

void TTabWidget::setSmallLogoFilename(const QString &smallLogoFilename)
{
    // Check that we haven't used this before.
    Q_ASSERT(_smallLogo == nullptr);

    // Save filename and load logo.
    _smallLogoFilename = smallLogoFilename;
    _smallLogo         = new QPixmap(_smallLogoFilename);
    _needRecalculate   = true;
}

void TTabWidget::resizeEvent(QResizeEvent *event)
{
    QTabWidget::resizeEvent(event);

    // We need to recalculate the widths.
    //
    // If this was a perfectly reusable class, we would also recalculate after
    // every ::tabInserted() and ::tabRemoved().  However, this is sufficient
    // for Tarsnap-GUI.
    _needRecalculate = true;
}

void TTabWidget::paintEvent(QPaintEvent *event)
{
    // Draw the normal elements of a QTabWidget.
    QTabWidget::paintEvent(event);

    // Update width calculation if necessary.
    if(_needRecalculate)
        recalculateWidth();

    // Bail if it's too narrow to draw either logo.
    if(_image == nullptr)
        return;

    // Draw the selected logo.
    QPainter p(this);
    p.drawPixmap(_image_x, 0, *_image);
}

void TTabWidget::recalculateWidth()
{
    // We don't need to call this again (unless something else changes).
    _needRecalculate = true;

    // Bail if we're missing either logo.
    if((!_largeLogo) || (!_smallLogo))
        return;

    // How much width is available?
    int remainingWidth = width() - tabBar()->width() - RIGHT_PADDING;

    // Pick which image (if any) to use.
    if(remainingWidth > _largeLogo->width())
        _image = _largeLogo;
    else if(remainingWidth > _smallLogo->width())
        _image = _smallLogo;
    else
    {
        _image = nullptr;
        // It doesn't matter what _image_x is in this case.
        return;
    }

    // How far along (width-wise) should we draw the logo?
    _image_x = width() - RIGHT_PADDING - _image->width();
}
