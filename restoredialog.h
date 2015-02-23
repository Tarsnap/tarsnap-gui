#ifndef RESTOREDIALOG_H
#define RESTOREDIALOG_H

#include <QDialog>

namespace Ui {
class RestoreDialog;
}

class RestoreDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RestoreDialog(QWidget *parent = 0);
    ~RestoreDialog();

private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void on_displayOptionsCheckBox_toggled(bool checked);

    void on_presetPathsCheckBox_toggled(bool checked);

private:
    Ui::RestoreDialog *_ui;
};

#endif // RESTOREDIALOG_H
