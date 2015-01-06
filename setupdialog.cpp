#include "setupdialog.h"
#include "ui_setupdialog.h"

#include <QDebug>
#include <QFileInfo>
#include <QFileDialog>
#include <QHostInfo>
#include <QStandardPaths>

SetupDialog::SetupDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetupDialog)
{
    ui->setupUi(this);

    connect(ui->welcomePageRadioButton, SIGNAL(clicked()), this, SLOT(skipToPage()));
    connect(ui->restorePageRadioButton, SIGNAL(clicked()), this, SLOT(skipToPage()));
    connect(ui->tarsnapPathPageRadioButton, SIGNAL(clicked()), this, SLOT(skipToPage()));
    connect(ui->newAccountPageRadioButton, SIGNAL(clicked()), this, SLOT(skipToPage()));
    connect(ui->donePageRadioButton, SIGNAL(clicked()), this, SLOT(skipToPage()));

    connect(ui->wizardStackedWidget, SIGNAL(currentChanged(int)), this, SLOT(wizardPageChanged(int)));

    connect(ui->welcomePageCancelButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->welcomePageProceedButton, SIGNAL(clicked()), this, SLOT(setNextPage()));

    connect(ui->tarsnapPathBrowseButton, SIGNAL(clicked()), this, SLOT(showTarsnapPathBrowse()));
    connect(ui->tarsnapPathLineEdit, SIGNAL(textChanged(QString)), this, SLOT(validateTarsnapPath(QString)));
    connect(ui->tarsnapPathProceedButton, SIGNAL(clicked()), this, SLOT(setNextPage()));

    connect(ui->restoreNoButton, SIGNAL(clicked()), this, SLOT(setNextPage()));
    connect(ui->restoreYesButton, SIGNAL(clicked()), this, SLOT(setNextPage()));

    _tarsnapKeysDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir keysDir(_tarsnapKeysDir);
    if(!keysDir.exists())
        keysDir.mkpath(_tarsnapKeysDir);

    _tarsnapCacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir cacheDir(_tarsnapCacheDir);
    if(!cacheDir.exists())
        cacheDir.mkpath(_tarsnapCacheDir);

    qDebug() << _tarsnapKeysDir << ::endl << _tarsnapCacheDir;


    findTarsnapInPath();
    ui->hostnameLineEdit->setText(QHostInfo::localHostName());

    ui->wizardStackedWidget->setCurrentWidget(ui->welcomePage);
}

SetupDialog::~SetupDialog()
{
    delete ui;
}

void SetupDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        _windowDragPos = event->pos();
    }
}

void SetupDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        QPoint diff = event->pos() - _windowDragPos;
        QPoint newpos = this->pos() + diff;
        this->move(newpos);
    }
}

void SetupDialog::wizardPageChanged(int)
{
    if(ui->wizardStackedWidget->currentWidget() == ui->welcomePage)
    {
        ui->welcomePageRadioButton->setChecked(true);
    }
    else
    if(ui->wizardStackedWidget->currentWidget() == ui->tarsnapPathPage)
    {
        ui->tarsnapPathPageRadioButton->setChecked(true);
    }
    else
    if(ui->wizardStackedWidget->currentWidget() == ui->restorePage)
    {
        ui->restorePageRadioButton->setChecked(true);
    }
    else
    if(ui->wizardStackedWidget->currentWidget() == ui->newAccountPage)
    {
        ui->newAccountPageRadioButton->setChecked(true);
    }
    else
    if(ui->wizardStackedWidget->currentWidget() == ui->donePage)
    {
        ui->donePageRadioButton->setChecked(true);
    }
}

void SetupDialog::skipToPage()
{
    if(sender() == ui->welcomePageRadioButton)
        ui->wizardStackedWidget->setCurrentWidget(ui->welcomePage);
    else if(sender() == ui->restorePageRadioButton)
        ui->wizardStackedWidget->setCurrentWidget(ui->restorePage);
    else if(sender() == ui->tarsnapPathPageRadioButton)
        ui->wizardStackedWidget->setCurrentWidget(ui->tarsnapPathPage);
    else if(sender() == ui->newAccountPageRadioButton)
        ui->wizardStackedWidget->setCurrentWidget(ui->newAccountPage);
    else if(sender() == ui->donePageRadioButton)
        ui->wizardStackedWidget->setCurrentWidget(ui->donePage);
}

void SetupDialog::setNextPage()
{
    if(ui->wizardStackedWidget->currentWidget() == ui->welcomePage)
    {
        if(ui->advancedSettingsCheckBox->isChecked() || _tarsnapCLIDir.isEmpty() || _tarsnapCacheDir.isEmpty() || _tarsnapKeysDir.isEmpty())
            ui->wizardStackedWidget->setCurrentWidget(ui->tarsnapPathPage);
        else
            ui->wizardStackedWidget->setCurrentWidget(ui->restorePage);
    }
    else
    if(ui->wizardStackedWidget->currentWidget() == ui->tarsnapPathPage)
    {
        ui->wizardStackedWidget->setCurrentWidget(ui->restorePage);
    }
    else
    if(ui->wizardStackedWidget->currentWidget() == ui->restorePage)
    {
        ui->wizardStackedWidget->setCurrentWidget(ui->newAccountPage);
    }
}

void SetupDialog::showTarsnapPathBrowse()
{
    QString tarsnapPath = QFileDialog::getExistingDirectory(this,
                                                            tr("Find Tarsnap client"),
                                                            "");
    ui->tarsnapPathLineEdit->setText(tarsnapPath);
}

bool SetupDialog::validateTarsnapPath(QString path)
{
    ui->tarsnapPathProceedButton->setEnabled(false);
    if ( !path.isEmpty() )
    {
        if ( !path.endsWith( "/" ) )
            path.append( "/" );
        QFileInfo candidate( path + "tarsnap" );
        if ( candidate.isFile() && candidate.exists() && candidate.isReadable() && candidate.isExecutable())
        {
            _tarsnapCLIDir = path;
            ui->tarsnapPathLineEdit->setText(_tarsnapCLIDir);
            ui->tarsnapPathProceedButton->setEnabled(true);
            return true;
        }
    }
    return false;
}

void SetupDialog::findTarsnapInPath()
{
    QStringList path = QString::fromUtf8(::getenv("PATH")).split(':', QString::SkipEmptyParts);
    qDebug() << path;
    auto dir = path.begin();
    while (dir != path.end())
    {
        if(validateTarsnapPath(*dir))
            return;
        ++dir;
    }
}

void SetupDialog::commitSettings()
{

}
