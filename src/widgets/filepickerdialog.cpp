#include "filepickerdialog.h"

FilePickerDialog::FilePickerDialog(QWidget *parent)
    : QDialog(parent)
{
    _ui.setupUi(this);
    connect(_ui.filePickerWidget, &FilePickerWidget::selectionChanged, [&]()
    {
        _ui.selectButton->setEnabled(!_ui.filePickerWidget->getSelectedUrls().isEmpty());
    });
    connect(_ui.selectButton, &QPushButton::clicked, this,
            &FilePickerDialog::accept);
}

FilePickerDialog::~FilePickerDialog()
{
}

QList<QUrl> FilePickerDialog::getSelectedUrls()
{
    return _ui.filePickerWidget->getSelectedUrls();
}

void FilePickerDialog::setSelectedUrls(QList<QUrl> urls)
{
    _ui.filePickerWidget->setSelectedUrls(urls);
}

void FilePickerDialog::selectUrl(QUrl url)
{
    _ui.filePickerWidget->selectUrl(url);
}
