#include "filepickerdialog.h"
#include "ui_filepickerdialog.h"

FilePickerDialog::FilePickerDialog(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::FilePickerDialog)
{
    _ui->setupUi(this);
    connect(_ui->cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_ui->selectButton, SIGNAL(clicked()), this, SLOT(accept()));
}

FilePickerDialog::~FilePickerDialog()
{
    delete _ui;
}

QList<QUrl> FilePickerDialog::getSelectedUrls()
{
    return _ui->filePickerWidget->getSelectedUrls();
}
