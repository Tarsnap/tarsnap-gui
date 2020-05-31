#include "archivestabwidget.h"

WARNINGS_DISABLE
#include <QAction>
#include <QComboBox>
#include <QCoreApplication>
#include <QCursor>
#include <QEvent>
#include <QEventLoop>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMenu>
#include <QWidget>
#include <Qt>

#include "ui_archivestabwidget.h"
WARNINGS_ENABLE

#include "basetask.h"
#include "persistentmodel/archive.h"
#include "widgets/archivelistwidget.h"
#include "widgets/archivewidget.h"

ArchivesTabWidget::ArchivesTabWidget(QWidget *parent)
    : QWidget(parent), _ui(new Ui::ArchivesTabWidget)
{
    // Ui initialization
    _ui->setupUi(this);
    updateKeyboardShortcutInfo();

    // Basic UI setup.
    _ui->archiveListWidget->setAttribute(Qt::WA_MacShowFocusRect, false);
    _ui->archiveDetailsWidget->hide();
    _ui->archivesFilterFrame->hide();

    // Context menu.
    _archiveListMenu = new QMenu(_ui->archiveListWidget);
    connect(_ui->archiveListWidget,
            &ArchiveListWidget::customContextMenuRequested, this,
            &ArchivesTabWidget::showArchiveListMenu);

    // Context menu actions.
    _ui->archiveListWidget->addAction(_ui->actionRefresh);
    _ui->archiveListWidget->addAction(_ui->actionInspect);
    _ui->archiveListWidget->addAction(_ui->actionDelete);
    _ui->archiveListWidget->addAction(_ui->actionRestore);
    connect(_ui->actionRefresh, &QAction::triggered, this,
            &ArchivesTabWidget::getArchives);
    connect(_ui->actionInspect, &QAction::triggered, _ui->archiveListWidget,
            &ArchiveListWidget::inspectSelectedItem);
    connect(_ui->actionDelete, &QAction::triggered, _ui->archiveListWidget,
            &ArchiveListWidget::deleteSelectedItems);
    connect(_ui->actionRestore, &QAction::triggered, _ui->archiveListWidget,
            &ArchiveListWidget::restoreSelectedItem);

    // Connections from the ArchiveListWidget.
    connect(_ui->archiveListWidget, &ArchiveListWidget::inspectArchive, this,
            &ArchivesTabWidget::displayInspectArchive);
    connect(_ui->archiveListWidget, &ArchiveListWidget::clearInspectArchive,
            this, &ArchivesTabWidget::hideInspectArchive);
    connect(_ui->archiveListWidget, &ArchiveListWidget::deleteArchives, this,
            &ArchivesTabWidget::deleteArchives);
    connect(_ui->archiveListWidget, &ArchiveListWidget::restoreArchive, this,
            &ArchivesTabWidget::restoreArchive);
    connect(_ui->archiveListWidget, &ArchiveListWidget::displayJobDetails,
            [this](const QString &jobRef) { emit displayJobDetails(jobRef); });
    connect(_ui->archiveListWidget, &ArchiveListWidget::countChanged,
            [this](int total, int visible) {
                _ui->archivesCountLabel->setText(
                    tr("Archives (%1/%2)").arg(visible).arg(total));
            });

    // Connections to the ArchiveListWidget.
    connect(this, &ArchivesTabWidget::archiveList, _ui->archiveListWidget,
            &ArchiveListWidget::setArchives);
    connect(this, &ArchivesTabWidget::addArchive, _ui->archiveListWidget,
            &ArchiveListWidget::addArchive);

    // Connections from the ArchiveDetailsWidget.
    connect(_ui->archiveDetailsWidget, &ArchiveDetailsWidget::restoreArchive,
            this, &ArchivesTabWidget::restoreArchive);
    connect(_ui->archiveDetailsWidget, &ArchiveDetailsWidget::hidden,
            _ui->archiveListWidget, &ArchiveListWidget::noInspect);
    connect(_ui->archiveDetailsWidget, &ArchiveDetailsWidget::taskRequested,
            this, &ArchivesTabWidget::taskRequested);
    connect(_ui->archiveDetailsWidget, &ArchiveDetailsWidget::jobClicked,
            [this](const QString &jobRef) { emit jobClicked(jobRef); });

    // Filtering.
    _ui->archiveListWidget->addAction(_ui->actionFilterArchives);
    _ui->archivesFilterButton->setDefaultAction(_ui->actionFilterArchives);
    connect(_ui->archivesFilter, &QComboBox::editTextChanged,
            _ui->archiveListWidget, &ArchiveListWidget::setFilter);
    connect(_ui->actionFilterArchives, &QAction::triggered, [this]() {
        _ui->archivesFilterFrame->setVisible(
            !_ui->archivesFilterFrame->isVisible());
        if(_ui->archivesFilter->isVisible())
            _ui->archivesFilter->setFocus();
        else
            _ui->archivesFilter->clearEditText();
    });
    connect(_ui->archivesFilter, &QComboBox::editTextChanged,
            _ui->archiveListWidget, &ArchiveListWidget::setFilter);
    connect(_ui->archivesFilter,
            static_cast<void (QComboBox::*)(int)>(
                &QComboBox::currentIndexChanged),
            [this]() { _ui->archiveListWidget->setFocus(); });
}

ArchivesTabWidget::~ArchivesTabWidget()
{
    delete _ui;
}

void ArchivesTabWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        _ui->retranslateUi(this);
        updateKeyboardShortcutInfo();
    }
    QWidget::changeEvent(event);
}

void ArchivesTabWidget::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Escape:
        // Apply to the ArchiveDetailsWidget, if possible...
        if(_ui->archiveDetailsWidget->isVisible())
        {
            _ui->archiveDetailsWidget->close();
            return;
        }
        // ... otherwise, apply the archive filter, if possible...
        if(_ui->archivesFilter->isVisible())
        {
            if(_ui->archivesFilter->currentText().isEmpty())
            {
                _ui->actionFilterArchives->trigger();
            }
            else
            {
                _ui->archivesFilter->clearEditText();
                _ui->archivesFilter->setFocus();
            }
            return;
        }
        // ... otherwise, use the default handling.
        break;
    }
    QWidget::keyPressEvent(event);
}

void ArchivesTabWidget::hideInspectArchive()
{
    _ui->archiveDetailsWidget->hide();
}

void ArchivesTabWidget::displayInspectArchive(ArchivePtr archive)
{
    // Get the size.
    if(archive->sizeTotal() == 0)
        emit loadArchiveStats(archive);

    // Get the file list.
    if(archive->contents().count() == 0)
        emit loadArchiveContents(archive);

    // Highlight the row in the ArchiveListWidget.
    _ui->archiveListWidget->selectArchive(archive);

    // Display Archive details.
    _ui->archiveDetailsWidget->setArchive(archive);
    if(!_ui->archiveDetailsWidget->isVisible())
    {
        _ui->archiveDetailsWidget->show();
        // Allow the event loop to update the geometry of the scroll area.
        // I haven't found a way to fix this without dropping back to the
        // main event loop.  :(
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    }

    // Make sure we can see it in the Archive List.  Must be after showing
    // the ArchiveDetailsWidget, otherwise the highlighted row might be
    // "visible" underneath the newly-expanded ArchiveDetailsWidget.
    _ui->archiveListWidget->ensureCurrentItemVisible();
}

void ArchivesTabWidget::showArchiveListMenu()
{
    // Construct menu.
    _archiveListMenu->clear();
    if(!_ui->archiveListWidget->selectedItems().isEmpty())
    {
        if(_ui->archiveListWidget->selectedItems().count() == 1)
        {
            _archiveListMenu->addAction(_ui->actionInspect);
            _archiveListMenu->addAction(_ui->actionRestore);
        }
        _archiveListMenu->addAction(_ui->actionDelete);
    }
    _archiveListMenu->addAction(_ui->actionRefresh);

    // Display menu.
    _archiveListMenu->popup(QCursor::pos());
}

void ArchivesTabWidget::updateKeyboardShortcutInfo()
{
    _ui->actionFilterArchives->setToolTip(
        _ui->actionFilterArchives->toolTip().arg(
            _ui->actionFilterArchives->shortcut().toString(
                QKeySequence::NativeText)));
    _ui->archivesFilter->setToolTip(_ui->archivesFilter->toolTip().arg(
        _ui->actionFilterArchives->shortcut().toString(
            QKeySequence::NativeText)));
}

void ArchivesTabWidget::displayInspectArchiveByRef(const QString &archiveName)
{
    ArchivePtr archive = _ui->archiveListWidget->findArchiveByName(archiveName);
    displayInspectArchive(archive);
}

void ArchivesTabWidget::updateIEC()
{
    _ui->archiveDetailsWidget->updateIEC();
    _ui->archiveListWidget->updateIEC();
}
