#include "statusbarwidget.h"

WARNINGS_DISABLE
#include <QAction>
#include <QEvent>
#include <QKeySequence>
#include <QToolButton>
#include <QVariant>
#include <QWidget>

#include "ui_statusbarwidget.h"
WARNINGS_ENABLE

#include "TBusyLabel.h"
#include "TSettings.h"

#include "tasks/tasks-defs.h"
#include "widgets/elidedclickablelabel.h"
#include "widgets/statisticsdialog.h"

StatusBarWidget::StatusBarWidget(QWidget *parent)
    : QWidget(parent),
      _ui(new Ui::StatusBarWidget),
      _statsDialog(new StatisticsDialog(this))
{
    // Ui initialization
    _ui->setupUi(this);
    updateUi();

    TSettings settings;
    _ui->simulationIcon->setVisible(
        settings.value("tarsnap/dry_run", DEFAULT_DRY_RUN).toBool());

    connect(_ui->busyLabel, &TBusyLabel::clicked, this,
            &StatusBarWidget::stopTasksRequested);

    _ui->expandJournalButton->setDefaultAction(_ui->actionShowJournal);
    connect(_ui->actionShowJournal, &QAction::toggled, this,
            &StatusBarWidget::journalToggleRequested);

    // Set up the StatisticsWidget
    _ui->statisticsButton->setPopup(_statsDialog);
}

StatusBarWidget::~StatusBarWidget()
{
    delete _ui;
}

void StatusBarWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        _ui->retranslateUi(this);
        updateUi();

        // Clear previous-language status message
        updateStatusMessage("", "");
    }
    QWidget::changeEvent(event);
}

void StatusBarWidget::updateUi()
{
    _ui->actionShowJournal->setToolTip(_ui->actionShowJournal->toolTip().arg(
        _ui->actionShowJournal->shortcut().toString(QKeySequence::NativeText)));
    // This label's tooltip will be empty if the GUI is not busy.
    if(!_ui->busyLabel->toolTip().isEmpty())
        _ui->busyLabel->setToolTip(_ui->busyLabel->toolTip().arg(
            _ui->actionStopTasks->shortcut().toString(
                QKeySequence::NativeText)));
}

void StatusBarWidget::updateStatusMessage(const QString &message,
                                          const QString &detail)
{
    _ui->statusBarLabel->setText(message);
    if(!detail.isEmpty())
        _ui->statusBarLabel->setToolTip(detail);
}

void StatusBarWidget::updateSimulationIcon(int state)
{
    if(state == Qt::Unchecked)
        _ui->simulationIcon->hide();
    else
        _ui->simulationIcon->show();
}

void StatusBarWidget::showBusy(bool busy)
{
    _ui->busyLabel->animate(busy);
    _ui->busyLabel->setEnabled(busy);
    if(busy)
    {
        _ui->busyLabel->setToolTip(
            QString(tr("Tarsnap is busy running chores, click to change that"
                       " <span style=\"color:gray;font-size:small\">%1</span>"))
                .arg(_ui->actionStopTasks->shortcut().toString(
                    QKeySequence::NativeText)));
    }
    else
        _ui->busyLabel->setToolTip("");
}

// We can't connect a slot to a slot (fair enough), so we pass this through.
void StatusBarWidget::overallStatsChanged(quint64 sizeTotal,
                                          quint64 sizeCompressed,
                                          quint64 sizeUniqueTotal,
                                          quint64 sizeUniqueCompressed,
                                          quint64 archiveCount)
{
    _statsDialog->overallStatsChanged(sizeTotal, sizeCompressed,
                                      sizeUniqueTotal, sizeUniqueCompressed,
                                      archiveCount);
}

// We can't connect a slot to a slot (fair enough), so we pass this through.
void StatusBarWidget::updateIEC()
{
    _statsDialog->updateIEC();
}
