#ifndef TTABWIDGET_H
#define TTABWIDGET_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
#include <QString>
#include <QTabWidget>
WARNINGS_ENABLE

/* Forward declaration(s). */
namespace Ui
{
class TTabWidget;
}
class QPaintEvent;
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
    Q_PROPERTY(QString largeLogoFilename MEMBER _largeLogoFilename WRITE
                   setLargeLogoFilename DESIGNABLE true)
    //! Filename of the small icon.  Can only be set once.
    Q_PROPERTY(QString smallLogoFilename MEMBER _smallLogoFilename WRITE
                   setSmallLogoFilename DESIGNABLE true)

public:
    //! Constructor.
    explicit TTabWidget(QWidget *parent = nullptr);
    ~TTabWidget() override;

    //! Set the filename of the large icon.
    void setLargeLogoFilename(const QString &largeLogoFilename);
    //! Set the filename of the small icon.
    void setSmallLogoFilename(const QString &smallLogoFilename);

protected:
    //! We need to recalculate the available width.
    void resizeEvent(QResizeEvent *event) override;
    //! Draw one of the logos in the top-right corner.
    void paintEvent(QPaintEvent *event) override;
    //! We have a new tab.
    void tabInserted(int index) override;
    //! We removed a tab.
    void tabRemoved(int index) override;

private:
    Ui::TTabWidget *_ui;

    int _image_x;

    // These are only here so that we can set them in the designer
    QString _largeLogoFilename;
    QString _smallLogoFilename;

    QPixmap *_largeLogo;
    QPixmap *_smallLogo;
    QPixmap *_image; // Always a pointer to an existing object (or nullptr).

    void recalculateWidth();
};

#endif // TTABWIDGET_H
