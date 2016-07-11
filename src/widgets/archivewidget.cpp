#include "archivewidget.h"
#include "utils.h"

#include <QCloseEvent>

#define EMPTY_TAR_ARCHIVE_BYTES 2000

ArchiveWidget::ArchiveWidget(QWidget *parent)
    : QWidget(parent), _contentsModel(this),
      _proxyModel(&_contentsModel)
{
    _ui.setupUi(this);
    QSettings settings;
    _useIECPrefixes = settings.value("app/iec_prefixes", false).toBool();

    _ui.hideButton->setToolTip(_ui.hideButton->toolTip()
                               .arg(QKeySequence(Qt::Key_Escape)
                                    .toString(QKeySequence::NativeText)));
    _ui.filterButton->setToolTip(_ui.filterButton->toolTip()
                               .arg(_ui.filterButton->shortcut()
                                    .toString(QKeySequence::NativeText)));
    _ui.filterComboBox->hide();

    _proxyModel.setDynamicSortFilter(false);
    _proxyModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
    _proxyModel.setSourceModel(&_contentsModel);
    _ui.archiveContentsTableView->setModel(&_proxyModel);

    connect(_ui.hideButton, &QPushButton::clicked, this, &ArchiveWidget::close);
    connect(_ui.archiveJobLabel, &ElidedLabel::clicked,
            [&]() { emit jobClicked(_archive->jobRef()); });
    connect(&_contentsModel, &FileTableModel::modelReset, this, [&]()
    {
        _ui.archiveContentsTableView->resizeColumnsToContents();
        _ui.archiveContentsLabel->setText(tr("Contents (%1)")
                                          .arg(_contentsModel.rowCount()));
    });
    connect(_ui.filterComboBox, &QComboBox::editTextChanged, &_proxyModel,
            &QSortFilterProxyModel::setFilterWildcard);
    connect(_ui.filterComboBox,
            static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this,
            [&](){_ui.archiveContentsTableView->setFocus();});
    connect(_ui.filterButton, &QPushButton::toggled, this, [&](const bool checked)
    {
        _ui.filterComboBox->setVisible(checked);
        if(checked)
            _ui.filterComboBox->setFocus();
        else
            _ui.filterComboBox->clearEditText();
    });
}

ArchiveWidget::~ArchiveWidget()
{
}

void ArchiveWidget::setArchive(ArchivePtr archive)
{
    if(_archive)
        disconnect(_archive.data(), &Archive::changed, this,
                   &ArchiveWidget::updateDetails);

    _archive = archive;

    if(_archive)
    {
        connect(_archive.data(), &Archive::changed, this,
                &ArchiveWidget::updateDetails);
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
        _ui.archiveNameLabel->setText(_archive->name());
        _ui.archiveDateLabel->setText(
            _archive->timestamp().toString(Qt::DefaultLocaleLongDate));
        if(_archive->jobRef().isEmpty())
        {
            _ui.archiveJobLabel->hide();
            _ui.archiveJobLabelField->hide();
            _ui.archiveIconLabel->setStyleSheet("image: url(:/icons/tarsnap-icon-big.png)");
        }
        else
        {
            _ui.archiveJobLabel->show();
            _ui.archiveJobLabelField->show();
            _ui.archiveJobLabel->setText(_archive->jobRef());
            _ui.archiveIconLabel->setStyleSheet("image: url(:/icons/hard-drive-big.png)");
        }
        _ui.archiveSizeLabel->setText(
            Utils::humanBytes(_archive->sizeTotal(), _useIECPrefixes));
        _ui.archiveSizeLabel->setToolTip(_archive->archiveStats());
        _ui.archiveUniqueDataLabel->setText(
            Utils::humanBytes(_archive->sizeUniqueCompressed(), _useIECPrefixes));
        _ui.archiveUniqueDataLabel->setToolTip(_archive->archiveStats());
        _ui.archiveCommandLineEdit->setText(_archive->command());
        _ui.archiveCommandLineEdit->setCursorPosition(0);
        if(_archive->truncated())
        {
            _ui.infoLabel->setText(tr("This archive is truncated,"
                                      " data may be incomplete"));
            _ui.infoLabel->show();
        }
        else if(_archive->contents().isEmpty()
                && (_archive->sizeTotal() < EMPTY_TAR_ARCHIVE_BYTES))
        {
            _ui.infoLabel->setText(tr("This archive looks empty,"
                                      " no file data may be contained"
                                      " besides the TAR header"));
            _ui.infoLabel->show();
        }
        else
            _ui.infoLabel->hide();
        _contentsModel.setArchive(_archive);
    }
}

void ArchiveWidget::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    setArchive(ArchivePtr()); // Release memory held by the contents widget
}

void ArchiveWidget::keyPressEvent(QKeyEvent *event)
{
    if((event->key() == Qt::Key_Escape) && _ui.filterComboBox->isVisible())
        _ui.filterButton->toggle();
    else
        QWidget::keyPressEvent(event);
}
