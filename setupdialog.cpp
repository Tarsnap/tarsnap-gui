#include "setupdialog.h"
#include "ui_setupdialog.h"

SetupDialog::SetupDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetupDialog)
{
    ui->setupUi(this);

    connect(ui->welcomePageRadioButton, SIGNAL(clicked()), this, SLOT(setCurrentPage()));
    connect(ui->restorePageRadioButton, SIGNAL(clicked()), this, SLOT(setCurrentPage()));
    connect(ui->tarsnapPathPageRadioButton, SIGNAL(clicked()), this, SLOT(setCurrentPage()));
    connect(ui->newAccountPageRadioButton, SIGNAL(clicked()), this, SLOT(setCurrentPage()));
    connect(ui->donePageRadioButton, SIGNAL(clicked()), this, SLOT(setCurrentPage()));

    ui->wizardPagesStackedWidget->setCurrentWidget(ui->welcomePage);
}

SetupDialog::~SetupDialog()
{
    delete ui;
}

void SetupDialog::setCurrentPage()
{
    if(sender() == ui->welcomePageRadioButton)
        ui->wizardPagesStackedWidget->setCurrentWidget(ui->welcomePage);
    else if(sender() == ui->restorePageRadioButton)
        ui->wizardPagesStackedWidget->setCurrentWidget(ui->restorePage);
    else if(sender() == ui->tarsnapPathPageRadioButton)
        ui->wizardPagesStackedWidget->setCurrentWidget(ui->tarsnapPathPage);
    else if(sender() == ui->newAccountPageRadioButton)
        ui->wizardPagesStackedWidget->setCurrentWidget(ui->newAccountPage);
    else if(sender() == ui->donePageRadioButton)
        ui->wizardPagesStackedWidget->setCurrentWidget(ui->donePage);
}
