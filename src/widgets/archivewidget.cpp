#include "archivewidget.h"

WARNINGS_DISABLE
#include <QAction>
#include <QComboBox>
#include <QCursor>
#include <QDateTime>
#include <QEvent>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QModelIndex>
#include <QModelIndexList>
#include <QSortFilterProxyModel>
#include <QStringList>
#include <QTableView>
#include <QVariant>
#include <Qt>

#include "ui_archivewidget.h"
WARNINGS_ENABLE

#include "TElidedLabel.h"

#include "messages/archiverestoreoptions.h"

#include "basetask.h"
#include "filetablemodel.h"
#include "humanbytes.h"
#include "persistentmodel/archive.h"
#include "widgets/elidedclickablelabel.h"
#include "widgets/restoredialog.h"

#define EMPTY_TAR_ARCHIVE_BYTES 2000

ArchiveDetailsWidget::ArchiveDetailsWidget(QWidget *parent)
    : QWidget(parent),
      _ui(new Ui::ArchiveDetailsWidget),
      _archive(nullptr),
      _contentsModel(new FileTableModel(this)),
      _proxyModel(new QSortFilterProxyModel(_contentsModel)),
      _fileMenu(new QMenu(this))
{
    _ui->setupUi(this);
    updateKeyboardShortcutInfo();

    // Set up filter UI.
    _ui->filterComboBox->hide();
    _proxyModel->setDynamicSortFilter(false);
    _proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    _proxyModel->setSourceModel(_contentsModel);
    _ui->archiveContentsTableView->setModel(_proxyModel);
    _ui->archiveContentsTableView->setContextMenuPolicy(Qt::CustomContextMenu);

    // Set up other UI.
    _fileMenu->addAction(_ui->actionRestoreFiles);

    // Connections for UI functionality.
    connect(_ui->archiveContentsTableView,
            &QTableView::customContextMenuRequested, this,
            &ArchiveDetailsWidget::showContextMenu);
    connect(_ui->actionRestoreFiles, &QAction::triggered, this,
            &ArchiveDetailsWidget::restoreFiles);
    connect(_ui->archiveContentsTableView, &QTableView::activated, this,
            &ArchiveDetailsWidget::restoreFiles);
    connect(_ui->hideButton, &QPushButton::clicked, this,
            &ArchiveDetailsWidget::close);
    connect(_contentsModel, &FileTableModel::taskRequested, this,
            &ArchiveDetailsWidget::taskRequested);
    connect(_ui->archiveJobLabel, &ElidedClickableLabel::clicked,
            [this]() { emit jobClicked(_archive->jobRef()); });

    // Connection to reset the model.
    connect(_contentsModel, &FileTableModel::modelReset, [this]() {
        _ui->archiveContentsTableView->resizeColumnsToContents();
        _ui->archiveContentsLabel->setText(
            tr("Contents (%1)").arg(_contentsModel->rowCount()));
    });

    // Connections for filtering
    connect(_ui->filterComboBox, &QComboBox::editTextChanged, _proxyModel,
            &QSortFilterProxyModel::setFilterWildcard);
    connect(_ui->filterComboBox,
            static_cast<void (QComboBox::*)(int)>(
                &QComboBox::currentIndexChanged),
            [this]() { _ui->archiveContentsTableView->setFocus(); });
    connect(_ui->filterButton, &QPushButton::toggled,
            [this](const bool checked) {
                _ui->filterComboBox->setVisible(checked);
                if(checked)
                    _ui->filterComboBox->setFocus();
                else
                    _ui->filterComboBox->clearEditText();
            });
}

ArchiveDetailsWidget::~ArchiveDetailsWidget()
{
    delete _proxyModel;
    delete _contentsModel;
    delete _ui;
}

void ArchiveDetailsWidget::setArchive(ArchivePtr archive)
{
    // Remove previous connections (if applicable).
    if(_archive)
    {
        disconnect(_archive.data(), &Archive::changed, this,
                   &ArchiveDetailsWidget::updateDetails);
        disconnect(_archive.data(), &Archive::purged, this,
                   &ArchiveDetailsWidget::close);
    }

    // Store pointer.
    _archive = archive;

    // Set up new connections and refresh the display.
    if(_archive)
    {
        connect(_archive.data(), &Archive::changed, this,
                &ArchiveDetailsWidget::updateDetails);
        connect(_archive.data(), &Archive::purged, this,
                &ArchiveDetailsWidget::close);
        updateDetails();
    }
    else
    {
        // No need to store info about the previous Archive.
        _contentsModel->reset();
    }
}

void ArchiveDetailsWidget::updateDetails()
{
    // Bail (if applicable).
    if(!_archive)
        return;

    // Show basic archive info.
    _ui->archiveNameLabel->setText(_archive->name());
    _ui->archiveDateLabel->setText(
        _archive->timestamp().toString(Qt::DefaultLocaleLongDate));

    // Show info about a linked Job (if applicable).
    if(_archive->jobRef().isEmpty())
    {
        _ui->archiveJobLabel->hide();
        _ui->archiveJobLabelField->hide();
        _ui->archiveIconLabel->setStyleSheet(
            "image: url(:/logos/tarsnap-icon-big.png)");
    }
    else
    {
        _ui->archiveJobLabel->show();
        _ui->archiveJobLabelField->show();
        _ui->archiveJobLabel->setText(_archive->jobRef());
        _ui->archiveIconLabel->setStyleSheet(
            "image: url(:/icons/hard-drive-big.png)");
    }

    // Show size info about the archive.
    _ui->archiveSizeLabel->setToolTip(_archive->archiveStats());
    _ui->archiveUniqueDataLabel->setToolTip(_archive->archiveStats());
    updateIEC();

    // Show other info about the archive.
    _ui->archiveCommandLineEdit->setText(_archive->command());
    _ui->archiveCommandLineEdit->setToolTip(
        _archive->command().prepend("<p>").append("</p>"));
    _ui->archiveCommandLineEdit->setCursorPosition(0);

    // Warn about partial archives.
    if(_archive->truncated())
    {
        _ui->infoLabel->setText(tr("This archive is truncated,"
                                   " data may be incomplete"));
        _ui->infoLabel->setToolTip(_archive->truncatedInfo());
        _ui->infoLabel->show();
    }
    else if(_archive->contents().isEmpty()
            && (_archive->sizeTotal() < EMPTY_TAR_ARCHIVE_BYTES))
    {
        // Warn about a potentially empty archive.
        _ui->infoLabel->setText(tr("This archive looks empty,"
                                   " no file data may be contained"
                                   " besides the TAR header"));
        _ui->infoLabel->show();
    }
    else
        _ui->infoLabel->hide();

    // Show files in the archive.
    _contentsModel->setArchive(_archive);
}

void ArchiveDetailsWidget::updateIEC()
{
    // Bail (if applicable).
    if(!_archive)
        return;

    // Format and display sizes.
    _ui->archiveSizeLabel->setText(humanBytes(_archive->sizeTotal()));
    _ui->archiveUniqueDataLabel->setText(
        humanBytes(_archive->sizeUniqueCompressed()));
}

void ArchiveDetailsWidget::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    setArchive(ArchivePtr()); // Release memory held by the contents widget
    emit hidden();
}

void ArchiveDetailsWidget::keyPressEvent(QKeyEvent *event)
{
    // Special handling for Escape with filtering.
    if((event->key() == Qt::Key_Escape) && _ui->filterComboBox->isVisible())
    {
        if(_ui->filterComboBox->currentText().isEmpty())
        {
            _ui->filterButton->toggle();
        }
        else
        {
            _ui->filterComboBox->clearEditText();
            _ui->filterComboBox->setFocus();
        }
    }
    else
    {
        // Normal handling.
        QWidget::keyPressEvent(event);
    }
}

void ArchiveDetailsWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        _ui->retranslateUi(this);
        updateKeyboardShortcutInfo();
        updateDetails();
    }
    QWidget::changeEvent(event);
}

void ArchiveDetailsWidget::showContextMenu()
{
    _fileMenu->popup(QCursor::pos());
}

void ArchiveDetailsWidget::restoreFiles()
{
    // Get selected items, and bail if there's none.
    QModelIndexList indexes =
        _ui->archiveContentsTableView->selectionModel()->selectedRows();
    if(indexes.isEmpty())
        return;

    // Convert items to filenames.
    QStringList files;
    for(const QModelIndex &index : indexes)
        files << index.data().toString();

    // Launch RestoreDialog.
    RestoreDialog *restoreDialog = new RestoreDialog(this, _archive, files);
    restoreDialog->displayTarOption(false);
    connect(restoreDialog, &RestoreDialog::accepted, [this, restoreDialog] {
        emit restoreArchive(restoreDialog->archive(),
                            restoreDialog->getOptions());
    });
    restoreDialog->show();
}

void ArchiveDetailsWidget::updateKeyboardShortcutInfo()
{
    _ui->hideButton->setToolTip(_ui->hideButton->toolTip().arg(
        QKeySequence(Qt::Key_Escape).toString(QKeySequence::NativeText)));
    _ui->filterButton->setToolTip(_ui->filterButton->toolTip().arg(
        _ui->filterButton->shortcut().toString(QKeySequence::NativeText)));
}
