#ifndef TPROGRESSWIDGET_H
#define TPROGRESSWIDGET_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
#include <QString>
#include <QWidget>
WARNINGS_ENABLE

/* Forward declaration(s). */
namespace Ui
{
class TProgressWidget;
}

/*!
 * \ingroup lib-widgets
 * \brief The TProgressWidget is a QWidget which display a "status" message, a
 * spinning "busy" icon, and a QProgressBar.
 */
class TProgressWidget : public QWidget
{
    Q_OBJECT

public:
    //! Constructor.
    explicit TProgressWidget(QWidget *parent = nullptr);
    ~TProgressWidget() override;

    //! Normal status message.
    void messageNormal(const QString &text);

    //! Error status message.
    void messageError(const QString &text);

    //! Display a spinning "busy" indicator.
    void setBusy(bool active);

    //! Clear the status message, busy indicator, and progress.
    void clear();

public slots:
    //! Set the value of the QProgressBar.  Goes from 0 to 100, inclusive.
    void setValue(int value);

private:
    Ui::TProgressWidget *_ui;
};

#endif // !TPROGRESSWIDGET_H
