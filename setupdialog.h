#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include <QDialog>
#include <QMouseEvent>

namespace Ui {
class SetupDialog;
}

class SetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetupDialog(QWidget *parent = 0);
    ~SetupDialog();

public slots:
    void wizardPageChanged(int);
    void skipToPage();
    void showAdvancedSetup(bool display);
    void setNextPage();
    void showTarsnapPathBrowse();
    void showTarsnapCacheBrowse();
    bool validateAdvancedSetupPage();
    bool validateTarsnapPath(QString path);
    bool validateTarsnapCache(QString path);
    void validateRegisterPage();
    void registerHaveKeyBrowse(QString url);
    void registerMachine();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    void findTarsnapInPath();
    void commitSettings();

private:
    Ui::SetupDialog *ui;
    QPoint  _windowDragPos;

    QString _tarsnapCLIDir;
    QString _tarsnapUser;
    QString _tarsnapPassword;
    QString _tarsnapCacheDir;
    QString _tarsnapKeysDir;
};

#endif // SETUPDIALOG_H
