#include "filepickerdialog.h"

WARNINGS_DISABLE
#include <QDir>
#include <QPushButton>
#include <QUrl>
#include <QVariant>

#include "ui_filepickerdialog.h"
WARNINGS_ENABLE

#include "TSettings.h"

#include "widgets/filepickerwidget.h"

/* Forward declaration(s). */
class QWidget;

FilePickerDialog::FilePickerDialog(QWidget *parent)
    : QDialog(parent), _ui(new Ui::FilePickerDialog)
{
    _ui->setupUi(this);
    // Enable the "select" button if, and only if, there are selected URLs.
    connect(_ui->filePickerWidget, &FilePickerWidget::selectionChanged,
            [this]() {
                _ui->selectButton->setEnabled(
                    !_ui->filePickerWidget->getSelectedUrls().isEmpty());
            });
    connect(_ui->selectButton, &QPushButton::clicked, this,
            &FilePickerDialog::accept);

    // Load last browsed file url.
    TSettings settings;
    _ui->filePickerWidget->setCurrentPath(
        settings.value("app/file_browse_last", QDir::homePath()).toString());
}

FilePickerDialog::~FilePickerDialog()
{
    // Save last browsed file url.
    TSettings settings;
    settings.setValue("app/file_browse_last",
                      _ui->filePickerWidget->getCurrentPath());
    delete _ui;
}

QList<QUrl> FilePickerDialog::getSelectedUrls()
{
    return _ui->filePickerWidget->getSelectedUrls();
}

void FilePickerDialog::setSelectedUrls(const QList<QUrl> &urls)
{
    _ui->filePickerWidget->setSelectedUrls(urls);
}

void FilePickerDialog::selectUrl(const QUrl &url)
{
    _ui->filePickerWidget->selectUrl(url);
}
