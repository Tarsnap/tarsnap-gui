#ifndef ABOUTWIDGET_H
#define ABOUTWIDGET_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QDialog>
#include <QObject>
#include <QString>
WARNINGS_ENABLE

/* Forward declaration(s). */
namespace Ui
{
class AboutDialog;
}
class QEvent;
class QWidget;

/*!
 * \ingroup widgets-main
 * \brief The AboutDialog is a QDialog which shows info about the app.
 */
class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    //! Constructor.
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog() override;

protected:
    //! Handles translation change of language.
    void changeEvent(QEvent *event) override;

private:
    Ui::AboutDialog *_ui;
};

#endif // ABOUTWIDGET_H
