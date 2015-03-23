#ifndef FILEPICKER_H
#define FILEPICKER_H

#include "ui_filepicker.h"
#include "customfilesystemmodel.h"

#include <QWidget>
#include <QFileSystemModel>
#include <QCompleter>

namespace Ui {
class FilePicker;
}

class FilePicker : public QWidget
{
    Q_OBJECT

public:
    explicit FilePicker(QWidget *parent = 0, QString startPath = "");
    ~FilePicker();

    QList<QUrl> getSelectedUrls();

protected:
    void keyPressEvent(QKeyEvent *event);

public slots:
    void updateFilter(QString filter);

private:
    Ui::FilePicker          *_ui;
    QString                 _startPath;
    CustomFileSystemModel    _model;
    QCompleter              _completer;
};

#endif // FILEPICKER_H
