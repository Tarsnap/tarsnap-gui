#include "archivestabwidget.h"

WARNINGS_DISABLE
#include <QWidget>

#include "ui_archivestabwidget.h"
WARNINGS_ENABLE

#include "ui_archivestabwidget.h"

#include <TSettings.h>

ArchivesTabWidget::ArchivesTabWidget(QWidget *parent)
    : QWidget(parent), _ui(new Ui::ArchivesTabWidget)
{
    // Ui initialization
    _ui->setupUi(this);

    _ui->archiveListWidget->setAttribute(Qt::WA_MacShowFocusRect, false);
    _ui->archiveDetailsWidget->hide();
    _ui->archivesFilterFrame->hide();

    connect(_ui->archiveListWidget,
            &ArchiveListWidget::customContextMenuRequested, this,
            &ArchivesTabWidget::showArchiveListMenu);

    _ui->archiveListWidget->addAction(_ui->actionRefresh);
    _ui->archiveListWidget->addAction(_ui->actionInspect);
    _ui->archiveListWidget->addAction(_ui->actionDelete);
    _ui->archiveListWidget->addAction(_ui->actionRestore);
    _ui->archiveListWidget->addAction(_ui->actionFilterArchives);
    _ui->archivesFilterButton->setDefaultAction(_ui->actionFilterArchives);

    connect(_ui->archiveListWidget, &ArchiveListWidget::inspectArchive, this,
            &ArchivesTabWidget::displayInspectArchive);
    connect(_ui->archiveListWidget, &ArchiveListWidget::clearInspectArchive,
            this, &ArchivesTabWidget::hideInspectArchive);
    connect(_ui->archiveListWidget, &ArchiveListWidget::deleteArchives, this,
            &ArchivesTabWidget::deleteArchives);
    connect(_ui->archiveListWidget, &ArchiveListWidget::restoreArchive, this,
            &ArchivesTabWidget::restoreArchive);
    connect(_ui->archiveDetailsWidget, &ArchiveWidget::restoreArchive, this,
            &ArchivesTabWidget::restoreArchive);
    connect(_ui->archiveDetailsWidget, &ArchiveWidget::hidden,
            _ui->archiveListWidget, &ArchiveListWidget::noInspect);

    connect(_ui->actionDelete, &QAction::triggered, _ui->archiveListWidget,
            &ArchiveListWidget::deleteSelectedItems);
    connect(_ui->actionRestore, &QAction::triggered, _ui->archiveListWidget,
            &ArchiveListWidget::restoreSelectedItem);
    connect(_ui->actionInspect, &QAction::triggered, _ui->archiveListWidget,
            &ArchiveListWidget::inspectSelectedItem);

    connect(_ui->archivesFilter, &QComboBox::editTextChanged,
            _ui->archiveListWidget, &ArchiveListWidget::setFilter);

    connect(_ui->actionFilterArchives, &QAction::triggered, [&]() {
        _ui->archivesFilterFrame->setVisible(
            !_ui->archivesFilterFrame->isVisible());
        if(_ui->archivesFilter->isVisible())
            _ui->archivesFilter->setFocus();
        else
            _ui->archivesFilter->clearEditText();
    });

    connect(_ui->archivesFilter, &QComboBox::editTextChanged,
            _ui->archiveListWidget, &ArchiveListWidget::setFilter);

    connect(_ui->archivesFilter, static_cast<void (QComboBox::*)(int)>(
                                     &QComboBox::currentIndexChanged),
            this, [&]() { _ui->archiveListWidget->setFocus(); });

    connect(_ui->archiveListWidget, &ArchiveListWidget::countChanged, this,
            [&](int total, int visible) {
                _ui->archivesCountLabel->setText(
                    tr("Archives (%1/%2)").arg(visible).arg(total));
            });

    connect(this, &ArchivesTabWidget::archiveList, _ui->archiveListWidget,
            &ArchiveListWidget::setArchives);
    connect(this, &ArchivesTabWidget::addArchive, _ui->archiveListWidget,
            &ArchiveListWidget::addArchive);

    connect(_ui->archiveDetailsWidget, &ArchiveWidget::jobClicked,
            [&](QString jobRef) { emit jobClicked(jobRef); });
    connect(_ui->archiveListWidget, &ArchiveListWidget::displayJobDetails,
            [&](QString jobRef) { emit displayJobDetails(jobRef); });

    updateUi();
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
        updateUi();
    }
    QWidget::changeEvent(event);
}

void ArchivesTabWidget::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Escape:
        if(_ui->archiveDetailsWidget->isVisible())
        {
            _ui->archiveDetailsWidget->close();
            return;
        }
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
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void ArchivesTabWidget::hideInspectArchive()
{
    _ui->archiveDetailsWidget->hide();
}

void ArchivesTabWidget::displayInspectArchive(ArchivePtr archive)
{
    if(archive->sizeTotal() == 0)
        emit loadArchiveStats(archive);

    if(archive->contents().count() == 0)
        emit loadArchiveContents(archive);

    _ui->archiveListWidget->selectArchive(archive);

    _ui->archiveDetailsWidget->setArchive(archive);
    if(!_ui->archiveDetailsWidget->isVisible())
    {
        _ui->archiveDetailsWidget->show();
        // Allow the event loop to update the geometry of the scroll area.
        // I haven't found a way to fix this without dropping back to the
        // main event loop.  :(
        QApplication::processEvents(0, 10);
    }

    _ui->archiveListWidget->ensureCurrentItemVisible();
}

void ArchivesTabWidget::showArchiveListMenu(const QPoint &pos)
{
    QPoint globalPos = _ui->archiveListWidget->viewport()->mapToGlobal(pos);
    QMenu  archiveListMenu(_ui->archiveListWidget);
    if(!_ui->archiveListWidget->selectedItems().isEmpty())
    {
        if(_ui->archiveListWidget->selectedItems().count() == 1)
        {
            archiveListMenu.addAction(_ui->actionInspect);
            archiveListMenu.addAction(_ui->actionRestore);
        }
        archiveListMenu.addAction(_ui->actionDelete);
    }
    archiveListMenu.addAction(_ui->actionRefresh);
    archiveListMenu.exec(globalPos);
}

void ArchivesTabWidget::updateUi()
{
    _ui->actionFilterArchives->setToolTip(
        _ui->actionFilterArchives->toolTip().arg(
            _ui->actionFilterArchives->shortcut().toString(
                QKeySequence::NativeText)));
    _ui->archivesFilter->setToolTip(_ui->archivesFilter->toolTip().arg(
        _ui->actionFilterArchives->shortcut().toString(
            QKeySequence::NativeText)));
}

void ArchivesTabWidget::displayInspectArchiveByRef(QString archiveName)
{
    ArchivePtr archive = _ui->archiveListWidget->findArchiveByName(archiveName);
    displayInspectArchive(archive);
}
