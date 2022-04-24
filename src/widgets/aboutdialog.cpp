#include "aboutdialog.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QEvent>
#include <QFlags>
#include <QLabel>
#include <QPushButton>
#include <QUrl>
#include <QWidget>
#include <Qt>

#include "ui_aboutdialog.h"
WARNINGS_ENABLE

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent), _ui(new Ui::AboutDialog)
{
    // Ui initialization
    _ui->setupUi(this);

    // Connect the Ok button
    connect(_ui->buttonBox, &QDialogButtonBox::accepted, this,
            &QDialog::accept);

    // Initialize About window
    _ui->versionLabel->setText(tr("GUI version ")
                               + QCoreApplication::applicationVersion());
    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint)
                   & ~Qt::WindowMaximizeButtonHint);
    connect(_ui->checkUpdateButton, &QPushButton::clicked, []() {
        QDesktopServices::openUrl(
            QUrl("https://github.com/Tarsnap/tarsnap-gui/releases"));
    });
}

AboutDialog::~AboutDialog()
{
    delete _ui;
}

void AboutDialog::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        _ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}
