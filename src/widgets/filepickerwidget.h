#ifndef FILEPICKERWIDGET_H
#define FILEPICKERWIDGET_H

#include "customfilesystemmodel.h"
#include "ui_filepickerwidget.h"

#include <QCompleter>
#include <QWidget>

class FilePickerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FilePickerWidget(QWidget *parent = nullptr);
    ~FilePickerWidget();

    void        reset();
    QList<QUrl> getSelectedUrls();
    void        setSelectedUrls(const QList<QUrl> &urls);
    void        selectUrl(QUrl url);
    bool        settingShowHidden();
    void        setSettingShowHidden(bool showHidden);
    bool        settingShowSystem();
    void        setSettingShowSystem(bool showSystem);
    bool        settingHideSymlinks();
    void        setSettingHideSymlinks(bool hideSymlinks);

public slots:
    void updateFilter(QString filter);
    void setCurrentPath(const QString path);

signals:
    void selectionChanged();
    void focusLost();
    void settingChanged();

protected:
    void keyPressEvent(QKeyEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::FilePickerWidget   _ui;
    CustomFileSystemModel  _model;
    QCompleter             _completer;
};

#endif // FILEPICKERWIDGET_H
