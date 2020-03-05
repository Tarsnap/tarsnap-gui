#include "archivewidget.h"

WARNINGS_DISABLE
#include <QCloseEvent>

#include "ui_archivewidget.h"
WARNINGS_ENABLE

#include "restoredialog.h"
#include "utils.h"

#define EMPTY_TAR_ARCHIVE_BYTES 2000

ArchiveWidget::ArchiveWidget(QWidget *parent)
    : QWidget(parent),
      _ui(new Ui::ArchiveWidget),
      _contentsModel(this),
      _proxyModel(&_contentsModel),
      _fileMenu(this)
{
    _ui->setupUi(this);
    _ui->filterComboBox->hide();
    updateUi();

    _proxyModel.setDynamicSortFilter(false);
    _proxyModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
    _proxyModel.setSourceModel(&_contentsModel);
    _ui->archiveContentsTableView->setModel(&_proxyModel);
    _ui->archiveContentsTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    _fileMenu.addAction(_ui->actionRestoreFiles);
    connect(_ui->archiveContentsTableView,
            &QTableView::customContextMenuRequested, this,
            &ArchiveWidget::showContextMenu);
    connect(_ui->actionRestoreFiles, &QAction::triggered, this,
            &ArchiveWidget::restoreFiles);
    connect(_ui->archiveContentsTableView, &QTableView::activated, this,
            &ArchiveWidget::restoreFiles);
    connect(_ui->hideButton, &QPushButton::clicked, this,
            &ArchiveWidget::close);
    connect(_ui->archiveJobLabel, &ElidedClickableLabel::clicked,
            [this]() { emit jobClicked(_archive->jobRef()); });
    connect(&_contentsModel, &FileTableModel::modelReset, [this]() {
        _ui->archiveContentsTableView->resizeColumnsToContents();
        _ui->archiveContentsLabel->setText(
            tr("Contents (%1)").arg(_contentsModel.rowCount()));
    });
    connect(_ui->filterComboBox, &QComboBox::editTextChanged, &_proxyModel,
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

ArchiveWidget::~ArchiveWidget()
{
    delete _ui;
}

void ArchiveWidget::setArchive(ArchivePtr archive)
{
    if(_archive)
    {
        disconnect(_archive.data(), &Archive::changed, this,
                   &ArchiveWidget::updateDetails);
        disconnect(_archive.data(), &Archive::purged, this,
                   &ArchiveWidget::close);
    }

    _archive = archive;

    if(_archive)
    {
        connect(_archive.data(), &Archive::changed, this,
                &ArchiveWidget::updateDetails);
        connect(_archive.data(), &Archive::purged, this, &ArchiveWidget::close);
        updateDetails();
    }
    else
    {
        _contentsModel.reset(); // reduce memory usage
    }
}

void ArchiveWidget::updateDetails()
{
    if(_archive)
    {
        _ui->archiveNameLabel->setText(_archive->name());
        _ui->archiveDateLabel->setText(
            _archive->timestamp().toString(Qt::DefaultLocaleLongDate));
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
        _ui->archiveSizeLabel->setText(
            Utils::humanBytes(_archive->sizeTotal()));
        _ui->archiveSizeLabel->setToolTip(_archive->archiveStats());
        _ui->archiveUniqueDataLabel->setText(
            Utils::humanBytes(_archive->sizeUniqueCompressed()));
        _ui->archiveUniqueDataLabel->setToolTip(_archive->archiveStats());
        _ui->archiveCommandLineEdit->setText(_archive->command());
        _ui->archiveCommandLineEdit->setToolTip(
            _archive->command().prepend("<p>").append("</p>"));
        _ui->archiveCommandLineEdit->setCursorPosition(0);
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
            _ui->infoLabel->setText(tr("This archive looks empty,"
                                       " no file data may be contained"
                                       " besides the TAR header"));
            _ui->infoLabel->show();
        }
        else
            _ui->infoLabel->hide();
        _contentsModel.setArchive(_archive);
    }
}

void ArchiveWidget::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    setArchive(ArchivePtr()); // Release memory held by the contents widget
    emit hidden();
}

void ArchiveWidget::keyPressEvent(QKeyEvent *event)
{
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
        QWidget::keyPressEvent(event);
    }
}

void ArchiveWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        _ui->retranslateUi(this);
        updateUi();
        updateDetails();
    }
    QWidget::changeEvent(event);
}

void ArchiveWidget::showContextMenu(const QPoint &pos)
{
    QPoint globalPos =
        _ui->archiveContentsTableView->viewport()->mapToGlobal(pos);
    _fileMenu.exec(globalPos);
}

void ArchiveWidget::restoreFiles()
{
    QModelIndexList indexes =
        _ui->archiveContentsTableView->selectionModel()->selectedRows();
    if(indexes.isEmpty())
        return;
    QStringList files;
    for(const QModelIndex &index : indexes)
        files << index.data().toString();

    RestoreDialog *restoreDialog = new RestoreDialog(this, _archive, files);
    restoreDialog->displayTarOption(false);
    restoreDialog->show();
    connect(restoreDialog, &RestoreDialog::accepted, [this, restoreDialog] {
        emit restoreArchive(restoreDialog->archive(),
                            restoreDialog->getOptions());
    });
}

void ArchiveWidget::updateUi()
{
    _ui->hideButton->setToolTip(_ui->hideButton->toolTip().arg(
        QKeySequence(Qt::Key_Escape).toString(QKeySequence::NativeText)));
    _ui->filterButton->setToolTip(_ui->filterButton->toolTip().arg(
        _ui->filterButton->shortcut().toString(QKeySequence::NativeText)));
}
