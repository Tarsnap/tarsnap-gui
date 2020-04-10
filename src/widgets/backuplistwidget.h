#ifndef BACKUPLISTWIDGET_H
#define BACKUPLISTWIDGET_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QListWidget>
#include <QUrl>
#include <QUuid>
WARNINGS_ENABLE

/* Forward declaration. */
class BaseTask;

/*!
 * \ingroup widgets-specialized
 * \brief The BackupListWidget is a QListWidget which lists
 * files and directories to be included in an on-demand backup.
 */
class BackupListWidget : public QListWidget
{
    Q_OBJECT

public:
    //! Constructor.
    explicit BackupListWidget(QWidget *parent = nullptr);
    ~BackupListWidget();

public slots:
    //! Add a single item to the backup list.
    //! \param url the path of the file or directory.
    void addItemWithUrl(QUrl url);
    //! Add multiple items to the backup list.
    //! \param urls a list of the paths of the files or directories.
    void addItemsWithUrls(QList<QUrl> urls);
    //! Returns a list of the paths of files or directories.
    QList<QUrl> itemUrls();
    //! Removes the currently-selected items.
    void removeItems();
    //! Calculates the number of items, and their total size.
    void recomputeListTotals();
    //! Clears any existing items, then adds the listed items.
    //! \param urls a list of the paths of the files or directories.
    void setItemsWithUrls(QList<QUrl> urls);

signals:
    //! The total number and size of items.
    //! \param count the total number.
    //! \param size the total size (in bytes).
    void itemTotals(quint64 count, quint64 size);
    //! A single item was added.
    //! \param url the path of the file or directory.
    void itemWithUrlAdded(QUrl url);
    //! We have a task to perform in the background.
    void taskRequested(BaseTask *task);
    //! We would like to cancel a task.
    void cancelTaskRequested(BaseTask *task, const QUuid uuid);

protected:
    //! Accepts a "drag-move" event if the dragged item contains urls.
    void dragMoveEvent(QDragMoveEvent *event);
    //! Accepts a "drag-enter" event if the dragged item contains urls.
    void dragEnterEvent(QDragEnterEvent *event);
    //! Adds the urls from the dragged item.
    void dropEvent(QDropEvent *event);
    //! Handles deleting items and clearing the selection.
    void keyPressEvent(QKeyEvent *event);
    //! Handles translation change of language.
    void changeEvent(QEvent *event);
};

#endif // BACKUPLISTWIDGET_H
