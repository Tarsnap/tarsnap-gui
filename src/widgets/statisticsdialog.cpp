#include "statisticsdialog.h"

WARNINGS_DISABLE
#include <QDialogButtonBox>
#include <QWidget>

#include "ui_statisticsdialog.h"
WARNINGS_ENABLE

#include "TSettings.h"

#include "humanbytes.h"

StatisticsDialog::StatisticsDialog(QWidget *parent)
    : QDialog(parent),
      _ui(new Ui::StatisticsDialog),
      _sizeTotal(0),
      _sizeUniqueCompressed(0),
      _storageSaved(0)
{
    // Ui initialization
    _ui->setupUi(this);
    updateUi();

    // "Ok" button.
    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this,
            &QDialog::accept);
}

StatisticsDialog::~StatisticsDialog()
{
    delete _ui;
}

void StatisticsDialog::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        _ui->retranslateUi(this);
        updateUi();
    }
    QWidget::changeEvent(event);
}

void StatisticsDialog::updateUi()
{
}

void StatisticsDialog::overallStatsChanged(quint64 sizeTotal,
                                           quint64 sizeCompressed,
                                           quint64 sizeUniqueTotal,
                                           quint64 sizeUniqueCompressed,
                                           quint64 archiveCount)
{
    // Calculate amount of data saved by Tarsnap.
    quint64 storageSaved = sizeTotal >= sizeUniqueCompressed
                               ? sizeTotal - sizeUniqueCompressed
                               : 0;

    // Set tooltip and labels.
    QString tooltip(tr("\t\tTotal size\tCompressed size\n"
                       "all archives\t%1\t\t%2\n"
                       "unique data\t%3\t\t%4")
                        .arg(sizeTotal)
                        .arg(sizeCompressed)
                        .arg(sizeUniqueTotal)
                        .arg(sizeUniqueCompressed));
    _ui->totalSizeStatLabel->setToolTip(tooltip);
    _ui->actualUsageStatLabel->setToolTip(tooltip);
    _ui->bytesEconomyStatLabel->setToolTip(tooltip);
    _ui->archivesCountStatLabel->setText(QString::number(archiveCount));

    // Set values which depend on "app/iec_prefixes"
    _sizeTotal            = sizeTotal;
    _sizeUniqueCompressed = sizeUniqueCompressed;
    _storageSaved         = storageSaved;
    updateIEC();
}

void StatisticsDialog::updateIEC()
{
    // Bail (if applicable).
    if(_sizeTotal == 0)
        return;

    // Format and display sizes.
    _ui->totalSizeStatLabel->setText(humanBytes(_sizeTotal));
    _ui->bytesEconomyStatLabel->setText(humanBytes(_storageSaved));
    _ui->actualUsageStatLabel->setText(humanBytes(_sizeUniqueCompressed));
}
