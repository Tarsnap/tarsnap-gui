#ifndef CONSOLELOGDIALOG_H
#define CONSOLELOGDIALOG_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QDialog>
#include <QObject>
#include <QString>
WARNINGS_ENABLE

/* Forward declaration(s). */
namespace Ui
{
class ConsoleLogDialog;
}
class TTextView;
class QWidget;

/*!
 * \ingroup widgets-main
 * \brief The ConsoleLogDialog is a QDialog which shows the console log.
 */
class ConsoleLogDialog : public QDialog
{
    Q_OBJECT

public:
    //! Constructor.
    explicit ConsoleLogDialog(QWidget *parent = nullptr);
    ~ConsoleLogDialog() override;

public slots:
    //! Append a log message.
    void appendLogString(const QString &text);

private:
    Ui::ConsoleLogDialog *_ui;

    TTextView *_consoleLog;
};

#endif // CONSOLELOGDIALOG_H
