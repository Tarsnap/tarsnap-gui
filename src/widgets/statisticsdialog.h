#ifndef STATISTICSDIALOG_H
#define STATISTICSDIALOG_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QDialog>
#include <QObject>
#include <QString>
WARNINGS_ENABLE

/* Forward declaration(s). */
namespace Ui
{
class StatisticsDialog;
}
class QEvent;

/*!
 * \ingroup widgets-main
 * \brief The StatisticsDialog is a QDialog which displays Tarsnap statistics.
 */
class StatisticsDialog : public QDialog
{
    Q_OBJECT

#ifdef QT_TESTLIB_LIB
    friend class TestMainWindow;
#endif

public:
    //! Constructor.
    explicit StatisticsDialog(QWidget *parent = nullptr);
    ~StatisticsDialog();

protected:
    //! Handles translation change of language.
    void changeEvent(QEvent *event);

private:
    Ui::StatisticsDialog *_ui;

    void updateUi();
};

#endif /* !STATISTICSDIALOG_H */
