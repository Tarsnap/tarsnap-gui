#include "archivewidget.h"

#include "utils.h"

#include <QHideEvent>

#define EMPTY_TAR_ARCHIVE_BYTES 2000

ArchiveWidget::ArchiveWidget(QWidget *parent)
    : QWidget(parent), _contentsModel(this)
{
    _ui.setupUi(this);
    _ui.archiveContentsTableView->setModel(&_contentsModel);
    QSettings settings;
    _useIECPrefixes = settings.value("app/iec_prefixes", false).toBool();

    _ui.hideButton->setToolTip(_ui.hideButton->toolTip()
                               .arg(QKeySequence(Qt::Key_Escape)
                                    .toString(QKeySequence::NativeText)));

    connect(_ui.hideButton, &QPushButton::clicked, this, &ArchiveWidget::hide);
    connect(_ui.archiveJobLabel, &ElidedLabel::clicked,
            [&]() { emit jobClicked(_archive->jobRef()); });
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
        _contentsModel.setFiles(_archive->contents());
        _ui.archiveContentsLabel->setText(tr("Contents (%1)")
                                          .arg(_contentsModel.rowCount()));
        _ui.archiveContentsTableView->resizeColumnsToContents();
    }
}

void ArchiveWidget::hideEvent(QHideEvent *event)
{
    if(!event->spontaneous())
        setArchive(ArchivePtr()); // Release memory held by the contents widget
}
