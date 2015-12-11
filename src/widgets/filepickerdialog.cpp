#include "filepickerdialog.h"
#include "ui_filepickerdialog.h"

FilePickerDialog::FilePickerDialog(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::FilePickerDialog)
{
    _ui->setupUi(this);
    connect(_ui->cancelButton, &QPushButton::clicked, this, &FilePickerDialog::reject);
    connect(_ui->selectButton, &QPushButton::clicked, this, &FilePickerDialog::accept);
}

FilePickerDialog::~FilePickerDialog()
{
    delete _ui;
}

QList<QUrl> FilePickerDialog::getSelectedUrls()
{
    return _ui->filePickerWidget->getSelectedUrls();
}
