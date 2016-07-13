#ifndef FILEPICKERWIDGET_H
#define FILEPICKERWIDGET_H

#include "customfilesystemmodel.h"
#include "ui_filepickerwidget.h"

#include <QCompleter>
#include <QWidget>

/*!
 * \ingroup widgets-specialized
 * \brief The FilePickerWidget filesystem is a QTreeView widget for selecting
 * files and directories; can be used standalone or inside a FilePickerDialog.
 */
class FilePickerWidget : public QWidget
{
    Q_OBJECT

public:
    //! Constructor.
    explicit FilePickerWidget(QWidget *parent = nullptr);
    ~FilePickerWidget();

    //! Resets the model, tree view, and the "last file browsed" setting.
    void        reset();

    //! Returns a list of the selected URLs.
    QList<QUrl> getSelectedUrls();
    //! Sets the list of selected URLs to be Qt::Checked in the underlying
    //! model.
    void        setSelectedUrls(const QList<QUrl> &urls);
    //! Sets an URL to be Qt::Checked in the underlying model.
    void        selectUrl(QUrl url);

    //! Returns the state of the "show hidden" checkbox.
    bool        settingShowHidden();
    //! Sets the state of the "show hidden" checkbox.
    void        setSettingShowHidden(bool showHidden);

    //! Returns the state of the "show system" checkbox.
    bool        settingShowSystem();
    //! Sets the state of the "show system " checkbox.
    void        setSettingShowSystem(bool showSystem);

    //! Returns the state of the "hide symbolic links" checkbox.
    bool        settingHideSymlinks();
    //! Sets the state of the "hide symbolic links" checkbox.
    void        setSettingHideSymlinks(bool hideSymlinks);

public slots:
    //! Updates a filter for filenames.  Directory names are unaffected.
    void updateFilter(QString filter);

    //! Sets the current selection in the QTreeView to the indicated path.
    void setCurrentPath(const QString path);

signals:
    //! The selected state of some paths in the underlying model has changed.
    void selectionChanged();

    //! The focus is no longer on this widget.
    void focusLost();

    //! One of the setting checkboxes has changed.
    void settingChanged();

protected:
    //! Used for handling the ESC key.
    void keyPressEvent(QKeyEvent *event);

    //! Used for determining if the focus was lost, and then emitting
    //! \ref focusLost and saving the currently-browsed path.
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::FilePickerWidget   _ui;
    CustomFileSystemModel  _model;
    QCompleter             _completer;
};

#endif // FILEPICKERWIDGET_H
