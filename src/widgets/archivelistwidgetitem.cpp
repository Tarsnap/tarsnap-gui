#include "archivelistwidgetitem.h"

#include "ui_archivelistwidgetitem.h"

#include "utils.h"

#define FIELD_WIDTH 6

ArchiveListWidgetItem::ArchiveListWidgetItem(ArchivePtr archive)
    : _ui(new Ui::ArchiveListWidgetItem), _widget(new QWidget)
{
    _ui->setupUi(_widget);
    // Send translation events to the widget.
    _widget->installEventFilter(this);
    updateUi();

    // Set up action connections.
    connect(_ui->deleteButton, &QToolButton::clicked, this,
            &ArchiveListWidgetItem::requestDelete);
    connect(_ui->inspectButton, &QToolButton::clicked, this,
            &ArchiveListWidgetItem::requestInspect);
    connect(_ui->restoreButton, &QToolButton::clicked, this,
            &ArchiveListWidgetItem::requestRestore);
    // Load the archive.
    setArchive(archive);
}

ArchiveListWidgetItem::~ArchiveListWidgetItem()
{
    delete _ui;
}

QWidget *ArchiveListWidgetItem::widget()
{
    return _widget;
}

ArchivePtr ArchiveListWidgetItem::archive() const
{
    return _archive;
}

void ArchiveListWidgetItem::setArchive(ArchivePtr archive)
{
    if(_archive)
    {
        disconnect(_archive.data(), &Archive::changed, this,
                   &ArchiveListWidgetItem::update);
        disconnect(_archive.data(), &Archive::purged, this,
                   &ArchiveListWidgetItem::removeItem);
    }

    _archive = archive;

    connect(_archive.data(), &Archive::changed, this,
            &ArchiveListWidgetItem::update, QUEUED);
    connect(_archive.data(), &Archive::purged, this,
            &ArchiveListWidgetItem::removeItem, QUEUED);

    QVector<QString> texts(3, "");
    QVector<QString> annotations(6, "");

    QString baseName = _archive->name();
    if(!_archive->jobRef().isEmpty()
       && _archive->name().startsWith(JOB_NAME_PREFIX))
    {
        texts[0]       = JOB_NAME_PREFIX;
        annotations[0] = "<font color=\"grey\">";
        annotations[1] = "</font>";
        baseName.remove(0, JOB_NAME_PREFIX.size());
    }
    if(baseName.size() > ARCHIVE_TIMESTAMP_FORMAT.size())
    {
        QString truncated;
        if(baseName.endsWith(QLatin1String(".part")))
        {
            truncated = QLatin1String(".part");
            baseName.chop(truncated.size());
        }
        QString   timestamp = baseName.right(ARCHIVE_TIMESTAMP_FORMAT.size());
        QDateTime validate =
            QDateTime::fromString(timestamp, ARCHIVE_TIMESTAMP_FORMAT);
        if(validate.isValid())
        {
            baseName.chop(timestamp.size());
            texts[2]       = QString("%1%2").arg(timestamp).arg(truncated);
            annotations[4] = "<font color=\"grey\">";
            annotations[5] = "</font>";
        }
    }
    texts[1] = baseName;
    _ui->nameLabel->setAnnotatedText(texts, annotations);
    _ui->nameLabel->setToolTip(_archive->name());
    QString detail(_archive->timestamp().toString(Qt::DefaultLocaleShortDate));
    if(_archive->sizeTotal() != 0)
    {
        QString size = Utils::humanBytes(_archive->sizeTotal(), FIELD_WIDTH);
        detail.prepend(size + "  ");
    }

    if(_archive->deleteScheduled())
    {
        _ui->detailLabel->setText(tr("(scheduled for deletion)"));
        _widget->setEnabled(false);
    }
    else
    {
        _ui->detailLabel->setText(detail);
        _widget->setEnabled(true);
    }

    _ui->detailLabel->setToolTip(_archive->archiveStats());
}

void ArchiveListWidgetItem::update()
{
    setArchive(_archive);
}

bool ArchiveListWidgetItem::eventFilter(QObject *obj, QEvent *event)
{
    if((obj == _widget) && (event->type() == QEvent::LanguageChange))
    {
        _ui->retranslateUi(_widget);
        updateUi();
        update();
        return true;
    }
    return false;
}

void ArchiveListWidgetItem::updateUi()
{
    // Display tooltips using platform-specific strings.
    _ui->inspectButton->setToolTip(_ui->inspectButton->toolTip().arg(
        _ui->actionInspect->shortcut().toString(QKeySequence::NativeText)));
    _ui->restoreButton->setToolTip(_ui->restoreButton->toolTip().arg(
        _ui->actionRestore->shortcut().toString(QKeySequence::NativeText)));
    _ui->deleteButton->setToolTip(_ui->deleteButton->toolTip().arg(
        _ui->actionDelete->shortcut().toString(QKeySequence::NativeText)));
}
