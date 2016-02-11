#include "archivewidget.h"
#include "ui_archivewidget.h"

ArchiveWidget::ArchiveWidget(QWidget *parent) :
    QWidget(parent),
    _ui(new Ui::ArchiveWidget)
{
    _ui->setupUi(this);
    QSettings settings;
    _useIECPrefixes = settings.value("app/iec_prefixes", false).toBool();

    connect(_ui->hideButton, &QPushButton::clicked, this, &ArchiveWidget::hide);
    connect(_ui->archiveJobLabel, &TextLabel::clicked, [&]() {
        emit jobClicked(_archive->jobRef());
    });
}

ArchiveWidget::~ArchiveWidget()
{
    delete _ui;
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
        _ui->archiveContentsPlainTextEdit->clear(); // reduce memory usage
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
        }
        else
        {
            _ui->archiveJobLabel->show();
            _ui->archiveJobLabelField->show();
            _ui->archiveJobLabel->setText(_archive->jobRef());
        }
        _ui->archiveSizeLabel->setText(
            Utils::humanBytes(_archive->sizeTotal(), _useIECPrefixes));
        _ui->archiveSizeLabel->setToolTip(_archive->archiveStats());
        _ui->archiveUniqueDataLabel->setText(
            Utils::humanBytes(_archive->sizeUniqueCompressed(),
                              _useIECPrefixes));
        _ui->archiveUniqueDataLabel->setToolTip(
            _archive->archiveStats());
        _ui->archiveCommandLineEdit->setText(_archive->command());
        _ui->archiveCommandLineEdit->setCursorPosition(0);
        QString contents = _archive->contents();
        _ui->archiveContentsLabel->setText(tr("Contents (%1)")
                                           .arg(QString::number(contents.count('\n'))));
        _ui->archiveContentsPlainTextEdit->setPlainText(contents);
    }
}
