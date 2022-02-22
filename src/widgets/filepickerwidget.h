#ifndef FILEPICKERWIDGET_H
#define FILEPICKERWIDGET_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QList>
#include <QObject>
#include <QString>
#include <QWidget>
WARNINGS_ENABLE

/* Forward declaration(s). */
namespace Ui
{
class FilePickerWidget;
}
class CustomFileSystemModel;
class QCompleter;
class QEvent;
class QKeyEvent;
class QUrl;

/*!
 * \ingroup widgets-specialized
 * \brief The FilePickerWidget filesystem is a QTreeView widget for selecting
 * files and directories; can be used standalone or inside a FilePickerDialog.
 */
class FilePickerWidget : public QWidget
{
    Q_OBJECT

#ifdef QT_TESTLIB_LIB
    friend class TestMainWindow;
#endif

public:
    //! Constructor.
    explicit FilePickerWidget(QWidget *parent = nullptr);
    ~FilePickerWidget() override;

    //! Resets the model, tree view, and the "last file browsed" setting.
    void reset();

    //! Returns the path of the currently-selected item.
    QString getCurrentPath();

    //! Returns a list of the selected URLs.
    QList<QUrl> getSelectedUrls();
    //! Sets the list of selected URLs to be Qt::Checked in the underlying
    //! model.
    void setSelectedUrls(const QList<QUrl> &urls);
    //! Sets an URL to be Qt::Checked in the underlying model.
    void selectUrl(const QUrl &url);

    //! Returns the state of the "show hidden" checkbox.
    bool settingShowHidden();
    //! Sets the state of the "show hidden" checkbox.
    void setSettingShowHidden(bool showHidden);

    //! Returns the state of the "show system" checkbox.
    bool settingShowSystem();
    //! Sets the state of the "show system " checkbox.
    void setSettingShowSystem(bool showSystem);

    //! Returns the state of the "hide symbolic links" checkbox.
    bool settingHideSymlinks();
    //! Sets the state of the "hide symbolic links" checkbox.
    void setSettingHideSymlinks(bool hideSymlinks);

public slots:
    //! Updates a filter for filenames.  Directory names are unaffected.
    void updateFilter(const QString &filter);

    //! Sets the current selection in the QTreeView to the indicated path.
    void setCurrentPath(const QString &path);

signals:
    //! The selected state of some paths in the underlying model has changed.
    void selectionChanged();

    //! One of the setting checkboxes has changed.
    void settingChanged();

protected:
    //! Used for handling the ESC key.
    void keyPressEvent(QKeyEvent *event) override;
    //! Used for handling the LanguageChange event
    void changeEvent(QEvent *event) override;

private:
    Ui::FilePickerWidget  *_ui;
    CustomFileSystemModel *_model;
    QCompleter            *_completer;
};

#endif // FILEPICKERWIDGET_H
