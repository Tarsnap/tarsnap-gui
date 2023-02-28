#include "TTabWidget.h"

WARNINGS_DISABLE
#include <QDebug>
#include <QLabel>
#include <QPixmap>
#include <QTabBar>
#include <QTabWidget>
#include <QUrl>

#include "ui_TTabWidget.h"
WARNINGS_ENABLE

class QResizeEvent;

#define RIGHT_PADDING 3

// I expected to find a function like this in Qt (maybe a static function in
// QResource?), but if it exists then I failed to find it.  For anybody
// looking at this code without the full context:
// - the Qt designer gives us a url pointing to the Qt Resource
// - I want to load that in a QPixmap
// - QPixmap only accepts a filename, not a QUrl.
static QString qurl_resource_to_string(const QUrl &url)
{
    Q_ASSERT(url.toString().startsWith("qrc"));
    return (url.toString().remove(0, 3));
}

TTabWidget::TTabWidget(QWidget *parent)
    : QTabWidget(parent),
      _ui(new Ui::TTabWidget),
      _largeLogo(new QPixmap),
      _smallLogo(new QPixmap),
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

void TTabWidget::setLargeLogoUrl(const QUrl &largeLogoUrl)
{
    // Save filename and load logo.
    _largeLogoUrl = largeLogoUrl;
    _largeLogo->load(qurl_resource_to_string(_largeLogoUrl));
    recalculateWidth();
}

void TTabWidget::setSmallLogoUrl(const QUrl &smallLogoUrl)
{
    // Save filename and load logo.
    _smallLogoUrl = smallLogoUrl;
    _smallLogo->load(qurl_resource_to_string(_smallLogoUrl));
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
    if((_largeLogo->height() == 0) || (_smallLogo->height() == 0))
        return;

    // Sanity check: both logos should be the same height.
    if(_largeLogo->height() != _smallLogo->height())
    {
        qDebug() << "TTabWidget: logos must be the same height:"
                 << _largeLogo->height() << _smallLogo->height();
        return;
    }

    // Sanity check: the logo height shouldn't be larger than the tabBar.
    if((tabBar()->height() > 0) && (_largeLogo->height() > tabBar()->height()))
    {
        qDebug() << "TTabWidget: logos are higher than the tabBar:"
                 << _largeLogo->height() << tabBar()->height();
        return;
    }

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
