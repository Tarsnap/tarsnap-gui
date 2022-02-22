#ifndef BACKUPLISTWIDGETITEM_H
#define BACKUPLISTWIDGETITEM_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QListWidgetItem>
#include <QObject>
#include <QUrl>
#include <QUuid>
WARNINGS_ENABLE

/* Forward declaration(s). */
namespace Ui
{
class BackupListWidgetItem;
}
class BaseTask;
class DirInfoTask;
class QMovie;
class QEvent;
class QWidget;

/*!
 * \ingroup widgets-specialized
 * \brief The BackupListWidgetItem is a QListWidgetItem which
 * displays information about a file or directory to be backed up.
 */
class BackupListWidgetItem : public QObject, public QListWidgetItem
{
    Q_OBJECT

public:
    //! Constructor.
    explicit BackupListWidgetItem();
    ~BackupListWidgetItem() override;

    //! Set the file or directory to include in this archive.
    void setUrl(const QUrl &url);

    //! Returns a widget with a graphical representation of this
    //! file or directory.
    QWidget *widget();

    //! Returns the url of this file or directory.
    QUrl url() const;

    //! Returns the number of files in this file or directory.
    quint64 count() const;

    //! Returns the total size of this file or directory.
    quint64 size() const;

    //! Reload the IEC prefix preference and re-display number(s).
    void updateIEC();

public slots:
    //! Displays the file or directory using the default
    //! desktop application.
    void browseUrl();

signals:
    //! The user requested to remove this item from the backup list.
    void requestDelete();
    //! The count and size of this directory have been (re)calculated.
    void requestUpdate();
    //! We have a task to perform in the background.
    void taskRequested(BaseTask *task);
    //! We would like to cancel a task.
    void cancelTaskRequested(BaseTask *task, const QUuid &uuid);

protected:
    //! Processes requests to switch language.
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void updateDirDetail(quint64 size, quint64 count);
    void cancelDirDetail();

private:
    Ui::BackupListWidgetItem *_ui;
    QWidget                  *_widget;
    QUrl                      _url;
    quint64                   _count;
    quint64                   _size;

    void startDirInfoTask();

    // This will be deleted by the TaskQueuer whenever it feels
    // like it, so do not rely on it being non-null.  Instead, we
    // use this pointer to indicate whether we think the task is
    // ongoing or not.
    DirInfoTask *_dirInfoTask;
    // The ID of the _dirInfoTask.
    QUuid _dirInfoTaskUuid;

    void updateKeyboardShortcutInfo();

    QMovie *_busyMovie;

    QString _size_prefix;
};

#endif // BACKUPLISTITEM_H
