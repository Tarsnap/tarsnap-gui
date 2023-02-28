#ifndef TTABWIDGET_H
#define TTABWIDGET_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
#include <QTabWidget>
#include <QUrl>
WARNINGS_ENABLE

/* Forward declaration(s). */
namespace Ui
{
class TTabWidget;
}
class QLabel;
class QPixmap;
class QResizeEvent;
class QWidget;

/*!
 * \ingroup lib-widgets
 * \brief The TTabWidget widget is a QTabWidget which will display one of two
 * images in the top-right corner.
 *
 * Both largeLogoFilename and smallLogoFilename must be set exactly once.
 * They are essentially arguments to the constructor, but using an indirect
 * route so that TTabWidget can be used in the Qt Designer.
 */
class TTabWidget : public QTabWidget
{
    Q_OBJECT

    //! Filename of the large icon.  Can only be set once.
    Q_PROPERTY(QUrl largeLogoUrl MEMBER _largeLogoUrl WRITE setLargeLogoUrl
                   DESIGNABLE true)
    //! Filename of the small icon.  Can only be set once.
    Q_PROPERTY(QUrl smallLogoUrl MEMBER _smallLogoUrl WRITE setSmallLogoUrl
                   DESIGNABLE true)

public:
    //! Constructor.
    explicit TTabWidget(QWidget *parent = nullptr);
    ~TTabWidget() override;

    //! Set the url of the large icon.
    void setLargeLogoUrl(const QUrl &largeLogoUrl);
    //! Set the url of the small icon.
    void setSmallLogoUrl(const QUrl &smallLogoUrl);

protected:
    //! We need to recalculate the available width.
    void resizeEvent(QResizeEvent *event) override;
    //! We have a new tab.
    void tabInserted(int index) override;
    //! We removed a tab.
    void tabRemoved(int index) override;

private:
    Ui::TTabWidget *_ui;

    // These are only here so that we can set them in the designer
    QUrl _largeLogoUrl;
    QUrl _smallLogoUrl;

    QPixmap *_largeLogo;
    QPixmap *_smallLogo;
    QLabel  *_cornerLabel;

    void recalculateWidth();
};

#endif // TTABWIDGET_H
