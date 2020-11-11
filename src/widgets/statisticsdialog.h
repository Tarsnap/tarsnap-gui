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
    ~StatisticsDialog() override;

    //! Update the global Tarsnap --print-stats values in the Settings tab.
    void overallStatsChanged(quint64 sizeTotal, quint64 sizeCompressed,
                             quint64 sizeUniqueTotal,
                             quint64 sizeUniqueCompressed,
                             quint64 archiveCount);

    //! Reload the IEC prefix preference and re-display number(s).
    void updateIEC();

protected:
    //! Handles translation change of language.
    void changeEvent(QEvent *event) override;

private:
    Ui::StatisticsDialog *_ui;

    void updateUi();

    // Values which depend on "app/iec_prefixes"
    quint64 _sizeTotal;
    quint64 _sizeUniqueCompressed;
    quint64 _storageSaved;
};

#endif /* !STATISTICSDIALOG_H */
