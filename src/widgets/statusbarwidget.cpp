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

#include "utils.h"
#include "widgets/elidedclickablelabel.h"

StatusBarWidget::StatusBarWidget(QWidget *parent)
    : QWidget(parent), _ui(new Ui::StatusBarWidget)
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
    _ui->busyLabel->setToolTip(_ui->busyLabel->toolTip().arg(
        _ui->actionStopTasks->shortcut().toString(QKeySequence::NativeText)));
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
}
