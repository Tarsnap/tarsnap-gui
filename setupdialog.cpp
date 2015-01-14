#include "setupdialog.h"
#include "ui_setupdialog.h"

#include <QDebug>
#include <QFileInfo>
#include <QFileDialog>
#include <QHostInfo>
#include <QStandardPaths>
#include <QDateTime>
#include <QDir>
#include <QSettings>

SetupDialog::SetupDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetupDialog),
    _haveKey(false)
{
    ui->setupUi(this);

    showAdvancedSetup(false);
    ui->errorLabel->hide();
    ui->hostKeyLabel->hide();
    ui->hostKeyLineEdit->hide();
    ui->hostHaveKeyLabel->hide();

    connect(ui->welcomePageRadioButton, SIGNAL(clicked()), this, SLOT(skipToPage()));
    connect(ui->restorePageRadioButton, SIGNAL(clicked()), this, SLOT(skipToPage()));
    connect(ui->advancedPageRadioButton, SIGNAL(clicked()), this, SLOT(skipToPage()));
    connect(ui->registerAccountPageRadioButton, SIGNAL(clicked()), this, SLOT(skipToPage()));
    connect(ui->donePageRadioButton, SIGNAL(clicked()), this, SLOT(skipToPage()));

    connect(ui->wizardStackedWidget, SIGNAL(currentChanged(int)), this, SLOT(wizardPageChanged(int)));

    // Welcome page
    connect(ui->advancedSetupCheckBox, SIGNAL(toggled(bool)), this, SLOT(showAdvancedSetup(bool)));
    connect(ui->welcomePageCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->welcomePageProceedButton, SIGNAL(clicked()), this, SLOT(setNextPage()));

    // Advanced setup page
    connect(ui->tarsnapPathBrowseButton, SIGNAL(clicked()), this, SLOT(showTarsnapPathBrowse()));
    connect(ui->tarsnapPathLineEdit, SIGNAL(textChanged(QString)), this, SLOT(validateAdvancedSetupPage()));
    connect(ui->tarsnapCacheBrowseButton, SIGNAL(clicked()), this, SLOT(showTarsnapCacheBrowse()));
    connect(ui->tarsnapCacheLineEdit, SIGNAL(textChanged(QString)), this, SLOT(validateAdvancedSetupPage()));
    connect(ui->advancedPageProceedButton, SIGNAL(clicked()), this, SLOT(setNextPage()));

    // Restore page
    connect(ui->restoreNoButton, SIGNAL(clicked()), this, SLOT(restoreNo()));
    connect(ui->restoreYesButton, SIGNAL(clicked()), this, SLOT(restoreYes()));

    // Register page
    connect(ui->tarsnapUserLineEdit, SIGNAL(textChanged(QString)), this, SLOT(validateRegisterPage()));
    connect(ui->tarsnapPasswordLineEdit, SIGNAL(textChanged(QString)), this, SLOT(validateRegisterPage()));
    connect(ui->hostNameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(validateRegisterPage()));
    connect(ui->hostKeyLineEdit, SIGNAL(textChanged(QString)), this, SLOT(validateRegisterPage()));
    connect(ui->hostHaveKeyLabel, SIGNAL(linkActivated(QString)), this, SLOT(registerHaveKeyBrowse(QString)));
    connect(ui->registerMachineButton, SIGNAL(clicked()), this, SLOT(registerMachine()));

    // Done page
    connect(ui->doneButton, SIGNAL(clicked()), this, SLOT(commitSettings()));

    _tarsnapKeysDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir keysDir(_tarsnapKeysDir);
    if(!keysDir.exists())
        keysDir.mkpath(_tarsnapKeysDir);

    _tarsnapCacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir cacheDir(_tarsnapCacheDir);
    if(!cacheDir.exists())
        cacheDir.mkpath(_tarsnapCacheDir);
    ui->tarsnapCacheLineEdit->setText(_tarsnapCacheDir);

    qDebug() << _tarsnapKeysDir << ::endl << _tarsnapCacheDir;

    findTarsnapInPath();

    ui->hostNameLineEdit->setText(QHostInfo::localHostName());

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
    if(ui->wizardStackedWidget->currentWidget() == ui->advancedPage)
    {
        ui->advancedPageRadioButton->setChecked(true);
    }
    else
    if(ui->wizardStackedWidget->currentWidget() == ui->restorePage)
    {
        ui->restorePageRadioButton->setChecked(true);
    }
    else
    if(ui->wizardStackedWidget->currentWidget() == ui->registerAccountPage)
    {
        ui->registerAccountPageRadioButton->setChecked(true);
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
    else if(sender() == ui->advancedPageRadioButton)
        ui->wizardStackedWidget->setCurrentWidget(ui->advancedPage);
    else if(sender() == ui->registerAccountPageRadioButton)
        ui->wizardStackedWidget->setCurrentWidget(ui->registerAccountPage);
    else if(sender() == ui->donePageRadioButton)
        ui->wizardStackedWidget->setCurrentWidget(ui->donePage);
}

void SetupDialog::showAdvancedSetup(bool display)
{
    ui->advancedPageRadioButton->setVisible(display);
}

void SetupDialog::setNextPage()
{
    if(ui->wizardStackedWidget->currentWidget() == ui->welcomePage)
    {
        if(ui->advancedSetupCheckBox->isChecked() || _tarsnapCLIDir.isEmpty()
           || _tarsnapCacheDir.isEmpty() || _tarsnapKeysDir.isEmpty())
        {
            showAdvancedSetup(true);
            ui->wizardStackedWidget->setCurrentWidget(ui->advancedPage);
            ui->advancedPageRadioButton->setEnabled(true);
        }
        else
        {
            ui->wizardStackedWidget->setCurrentWidget(ui->restorePage);
            ui->restorePageRadioButton->setEnabled(true);
        }
    }
    else if(ui->wizardStackedWidget->currentWidget() == ui->advancedPage)
    {
        ui->wizardStackedWidget->setCurrentWidget(ui->restorePage);
        ui->restorePageRadioButton->setEnabled(true);
    }
    else if(ui->wizardStackedWidget->currentWidget() == ui->restorePage)
    {
        ui->wizardStackedWidget->setCurrentWidget(ui->registerAccountPage);
        ui->registerAccountPageRadioButton->setEnabled(true);
    }
    else if(ui->wizardStackedWidget->currentWidget() == ui->registerAccountPage)
    {
        ui->wizardStackedWidget->setCurrentWidget(ui->donePage);
        ui->donePageRadioButton->setEnabled(true);
    }
}

void SetupDialog::showTarsnapPathBrowse()
{
    QString tarsnapPath = QFileDialog::getExistingDirectory(this,
                                                            tr("Find Tarsnap client"),
                                                            "");
    ui->tarsnapPathLineEdit->setText(tarsnapPath);
}

void SetupDialog::showTarsnapCacheBrowse()
{
    QString tarsnapCacheDir = QFileDialog::getExistingDirectory(this,
                                                            tr("Tarsnap cache location"),
                                                            _tarsnapCacheDir);
    ui->tarsnapCacheLineEdit->setText(tarsnapCacheDir);
}

bool SetupDialog::validateAdvancedSetupPage()
{
    bool result = false;

    if(validateTarsnapPath(ui->tarsnapPathLineEdit->text())
       && validateTarsnapCache(ui->tarsnapCacheLineEdit->text()))
        result = true;

    ui->advancedPageProceedButton->setEnabled(result);
    return result;
}

bool SetupDialog::validateTarsnapPath(QString path)
{
    bool result = false;
    if ( !path.isEmpty() )
    {
        if ( !path.endsWith( "/" ) )
            path.append( "/" );
        QFileInfo candidate( path + "tarsnap" );
        if ( candidate.isFile() && candidate.exists() && candidate.isReadable()
             && candidate.isExecutable())
        {
            _tarsnapCLIDir = path;
            result = true;
        }
    }
    return result;
}

bool SetupDialog::validateTarsnapCache(QString path)
{
    bool result = false;
    if(!path.isEmpty())
    {
        QFileInfo candidate(path);
        if(candidate.exists() && candidate.isDir() && candidate.isWritable())
        {
            _tarsnapCacheDir = candidate.absoluteFilePath();
            result = true;
        }
    }
    return result;
}

void SetupDialog::restoreNo()
{
    _haveKey = false;
    ui->hostKeyLabel->hide();
    ui->hostKeyLineEdit->hide();
    ui->hostHaveKeyLabel->hide();
    ui->tarsnapUserLabel->show();
    ui->tarsnapUserLineEdit->show();
    ui->tarsnapPasswordLabel->show();
    ui->tarsnapPasswordLineEdit->show();
    ui->registerPageInfoLabel->setText(tr("Please use your Tarsnap account credentials. Don't have an account? Register one <a href=\"http://tarsnap.com\">here</a>."));
    ui->errorLabel->clear();
    setNextPage();
}

void SetupDialog::restoreYes()
{
    _haveKey = true;
    ui->tarsnapUserLabel->hide();
    ui->tarsnapUserLineEdit->hide();
    ui->tarsnapPasswordLabel->hide();
    ui->tarsnapPasswordLineEdit->hide();
    ui->hostKeyLabel->show();
    ui->hostKeyLineEdit->show();
    ui->hostHaveKeyLabel->show();
    ui->registerPageInfoLabel->setText(tr("Please use your existing machine key."));
    ui->errorLabel->clear();
    setNextPage();
}

void SetupDialog::validateRegisterPage()
{
    bool result = false;
    if(_haveKey)
    {
        // user specified key
        QFileInfo hostKeyFile(ui->hostKeyLineEdit->text());
        if(!ui->hostNameLineEdit->text().isEmpty()
           && hostKeyFile.exists() && hostKeyFile.isFile() && hostKeyFile.isReadable())
            result = true;
    }
    else
    {
        if(!ui->tarsnapUserLineEdit->text().isEmpty()
           && !ui->tarsnapPasswordLineEdit->text().isEmpty()
           && !ui->hostNameLineEdit->text().isEmpty())
        {
            result = true;
        }
    }

    ui->registerMachineButton->setEnabled(result);
}

void SetupDialog::registerHaveKeyBrowse(QString url)
{
    Q_UNUSED(url);
    QString existingMachineKey = QFileDialog::getOpenFileName(this
                                                              ,tr("Browse for existing machine key"));
    ui->hostKeyLineEdit->setText(existingMachineKey);
}

void SetupDialog::registerMachine()
{
    ui->errorLabel->clear();
    if(_haveKey)
    {
        _tarsnapKeyFile = ui->hostKeyLineEdit->text();
    }
    else
    {
        _tarsnapKeyFile = _tarsnapKeysDir + QDir::separator() + ui->hostNameLineEdit->text()
                          + "_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    }

    qDebug() << "Registration details >>\n " << _tarsnapCLIDir << ::endl
             << _tarsnapKeysDir << ::endl << _tarsnapKeyFile << ::endl
             << _tarsnapCacheDir;

    emit registerMachine(ui->tarsnapUserLineEdit->text(), ui->tarsnapPasswordLineEdit->text()
                         , ui->hostNameLineEdit->text(), _tarsnapKeyFile);
}

void SetupDialog::registerMachineStatus(JobManager::JobStatus status, QString reason)
{
    switch(status)
    {
        case JobManager::Completed:
            ui->errorLabel->clear();
            ui->doneButton->setEnabled(true);
            setNextPage();
            break;
        case JobManager::Failed:
            ui->errorLabel->setText(reason);
            ui->errorLabel->show();
            resize(sizeHint());
            break;
    }
}

void SetupDialog::findTarsnapInPath()
{
    // Maybe use QStandardPaths::​findExecutable instead of manual PATH search
    QStringList path = QString::fromUtf8(::getenv("PATH")).split(':', QString::SkipEmptyParts);
    qDebug() << path;
    auto dir = path.begin();
    while (dir != path.end())
    {
        if(validateTarsnapPath(*dir))
        {
            ui->tarsnapPathLineEdit->setText(_tarsnapCLIDir);
            return;
        }
        ++dir;
    }
}

void SetupDialog::commitSettings()
{
    QSettings settings;

    settings.setValue("app/appdata", _tarsnapKeysDir);
    settings.setValue("tarsnap/path", _tarsnapCLIDir);
    settings.setValue("tarsnap/cache", _tarsnapCacheDir);
    settings.setValue("tarsnap/key", _tarsnapKeyFile);
    settings.setValue("tarsnap/user", ui->tarsnapUserLineEdit->text());
    settings.setValue("tarsnap/machine", ui->hostNameLineEdit->text());
    settings.sync();
    qDebug() << settings.fileName();
    accept();
}
