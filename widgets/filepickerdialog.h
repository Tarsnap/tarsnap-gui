#ifndef FILEPICKERDIALOG_H
#define FILEPICKERDIALOG_H

#include <QDialog>
#include <QUrl>

namespace Ui {
class FilePickerDialog;
}

class FilePickerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FilePickerDialog(QWidget *parent = 0);
    ~FilePickerDialog();

    QList<QUrl> getSelectedUrls();

protected:
    void keyPressEvent(QKeyEvent *event){Q_UNUSED(event);}

private:
    Ui::FilePickerDialog *_ui;
};

#endif // FILEPICKERDIALOG_H
