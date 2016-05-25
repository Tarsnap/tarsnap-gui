#ifndef FILEPICKERDIALOG_H
#define FILEPICKERDIALOG_H

#include "ui_filepickerdialog.h"

#include <QDialog>
#include <QUrl>

class FilePickerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FilePickerDialog(QWidget *parent = nullptr);
    ~FilePickerDialog();

    QList<QUrl> getSelectedUrls();
    void setSelectedUrls(QList<QUrl> urls);
    void selectUrl(QUrl url);

private:
    Ui::FilePickerDialog _ui;
};

#endif // FILEPICKERDIALOG_H
