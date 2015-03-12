#ifndef FILEPICKER_H
#define FILEPICKER_H

#include "ui_filepicker.h"

#include <QDialog>
#include <QFileSystemModel>

namespace Ui {
class FilePicker;
}

class FilePicker : public QDialog
{
    Q_OBJECT

public:
    explicit FilePicker(QWidget *parent = 0);
    ~FilePicker();

    QStringList getSelectedUris();

protected:
    void keyReleaseEvent(QKeyEvent *event);

public slots:
    void updateFilter(QString filter);

private:
    Ui::FilePicker      *_ui;
    QFileSystemModel    _model;
};

#endif // FILEPICKER_H
