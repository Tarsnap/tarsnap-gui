#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include <taskmanager.h>
#include "ui_setupdialog.h"

#include <QDialog>
#include <QMovie>

class SetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetupDialog(QWidget *parent = nullptr);
    ~SetupDialog();

public slots:
    // TaskManager responses
    void registerMachineStatus(TaskStatus status, QString reason);
    void setTarsnapVersion(QString versionString);
    void updateLoadingAnimation(bool idle);

signals:
    void requestRegisterMachine(QString user, QString password, QString machine,
                                QString key, QString tarsnapPath,
                                QString cachePath);
    void getTarsnapVersion(QString tarsnapPath);

private slots:
    void wizardPageChanged(int);
    void skipToPage();
    void showTarsnapPathBrowse();
    void showTarsnapCacheBrowse();
    void showAppDataBrowse();
    bool validateAdvancedSetupPage();
    void restoreNo();
    void restoreYes();
    bool validateRegisterPage();
    void registerHaveKeyBrowse();
    void backButtonClicked();
    void nextButtonClicked();

private:
    Ui::SetupDialog  _ui;
    QPoint           _windowDragPos;
    QMovie           _loadingAnimation;

    QString _tarsnapDir;
    QString _tarsnapVersion;
    QString _tarsnapCacheDir;
    QString _appDataDir;
    QString _tarsnapKeyFile;

    void setNextPage();
    void registerMachine();
    void commitSettings(bool skipped = false);
};

#endif // SETUPDIALOG_H
