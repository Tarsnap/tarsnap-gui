#include "archivewidget.h"

#include "utils.h"

ArchiveWidget::ArchiveWidget(QWidget *parent)
    : QWidget(parent)
{
    _ui.setupUi(this);
    QSettings settings;
    _useIECPrefixes = settings.value("app/iec_prefixes", false).toBool();

    _ui.hideButton->setToolTip(_ui.hideButton->toolTip()
                               .arg(QKeySequence(Qt::Key_Escape)
                                    .toString(QKeySequence::NativeText)));

    connect(_ui.hideButton, &QPushButton::clicked, this, &ArchiveWidget::hide);
    connect(_ui.archiveJobLabel, &TextLabel::clicked,
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
        _ui.archiveContentsPlainTextEdit->clear(); // reduce memory usage
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
        _ui.truncatedLabel->setVisible(_archive->truncated());
        QString contents = _archive->contents();
        _ui.archiveContentsLabel->setText(
            tr("Contents (%1)").arg(QString::number(contents.count('\n'))));
        _ui.archiveContentsPlainTextEdit->setPlainText(contents);
    }
}
